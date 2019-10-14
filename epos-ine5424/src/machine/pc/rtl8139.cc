// EPOS PC AMD PCNet II (Am79C970A) Ethernet NIC Mediator Implementation

#include <machine/machine.h>
#include <machine/pc/rtl8139.h>
#include <process.h>
#include <system.h>
#include <time.h>
#include <netservice.h>

__BEGIN_SYS

// Class attributes
RTL8139::Device RTL8139::_devices[UNITS];


// Methods
RTL8139::~RTL8139()
{
    db<RTL8139>(TRC) << "~RTL8139()" << endl;
}


int RTL8139::send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
{
    Buffer * buf = _tx_buffer[_tx_head];

    db<RTL8139>(TRC) << "RTL8139 will send, _tx_head " << _tx_head << endl;

    while (!buf->lock()) {
        db<RTL8139>(TRC) << "RTL8139 waiting to send, _tx_head " << _tx_head << endl;
        _waiting_to_send = Thread::self();
        _waiting_to_send->suspend();
    }

    new (buf->frame()) Frame(_address, dst, prot, data, size);
    unsigned short status = sizeof(Frame) & 0xfff;

    CPU::out32(_io_port + TRSTART  + _tx_head * 4, (long unsigned int) _tx_base_phy[_tx_head]);
    CPU::out32(_io_port + TRSTATUS + _tx_head * 4, status);

    _tx_head = (_tx_head + 1) % TX_BUFFER_NR;

    db<RTL8139>(TRC) << "RTL8139 sent, _tx_head now at " << _tx_head << endl;

    return size;
}


void RTL8139::receive()
{
    db<RTL8139>(TRC) << "RTL8139 receiving" << endl;


    unsigned int * rx = (unsigned int *) (_rx_buffer + _rx_read);
    unsigned int header = *rx;
    unsigned int packet_len = (header >> 16);
    unsigned int status = header & 0xffff;
    rx++;

    db<RTL8139>(TRC) << "rx_head=" << rx << endl;
    db<RTL8139>(TRC) << "packet_len=" << packet_len << endl;
    db<RTL8139>(TRC) << "status=" << status << endl;

    Frame * frame = reinterpret_cast<Frame *>(rx);
    db<RTL8139>(TRC) << "frame src " << frame->src() << endl;

    Buffer * buf = new Buffer(this, packet_len);

    // copy data to application
    memcpy(buf->frame(), frame, sizeof(Frame));
    db<RTL8139>(TRC) << "buffer src " << ((Frame*) buf->frame())->src() << endl;

    // update CAPR
    _rx_read += packet_len + 4;
    _rx_read =  (_rx_read + 3) & ~3;

    if (_rx_read > RX_NO_WRAP_SIZE) {
        _rx_read -= RX_NO_WRAP_SIZE;
        db<RTL8139>(TRC) << "\tWRAPPED " << _rx_read << endl;
    }

    CPU::out16(_io_port + CAPR, _rx_read - 0x10);
    db<RTL8139>(TRC) << "out(CAPR, " << _rx_read - 0x10<< ")" << endl;
    db<RTL8139>(TRC) << "CBR " << CPU::in16(_io_port + CBR) << endl;

    NetService::insert_buffer(buf);
}

RTL8139::Buffer * RTL8139::alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload)
{
    return nullptr;
}


int RTL8139::send(Buffer * buf)
{
    return 0;
}


void RTL8139::free(Buffer * buf) {}


void RTL8139::reset()
{

    db<RTL8139>(TRC) << "RTL8139::reset()" << endl;

    // Power on the NIC
    CPU::out8(_io_port + CONFIG_1, POWER_ON);

    db<RTL8139>(TRC) << "DMA Buffer is at phy " << _dma_buf->phy_address() << " | log " << _dma_buf->log_address() << endl;

    // Reset the device
    CPU::out8(_io_port + CMD, RESET);

    // Wait for RST bit to be low
    while (CPU::in8(_io_port + CMD) & RESET) {}

    // Tell the NIC where the receive buffer starts
    CPU::out32(_io_port + RBSTART, _dma_buf->phy_address());

    // Configure interrupt mask to transmit OK and receive OK events
    CPU::out16(_io_port + IMR, TOK | ROK);

    // Configure which packets should be received, don't wrap around buffer
    CPU::out32(_io_port + RCR, ACCEPT_ANY | WRAP);

    // Receive enable & transmit enable
    CPU::out8(_io_port  + CMD, TE | RE);

    // Set MAC address
    for (int i = 0; i < 6; i++) _address[i] = CPU::in8(_io_port + MAC0_5 + i);

    db<RTL8139>(TRC) << "RBSTART is " << CPU::in32(_io_port + RBSTART) << endl;

}

void RTL8139::handle_int()
{
    /* An interrupt usually means a single frame was transmitted or received,
     * but in certain cases it might handle several frames or none at all.
     */
    unsigned short status = CPU::in16(_io_port + ISR);
    db<RTL8139>(WRN) << "INTERRUPT STATUS " << status << endl;

    if (status & TOK) {
        // Transmit completed successfully, release descriptor
        db<RTL8139>(WRN) << "TOK" << endl;
        bool transmitted = false;
        for (unsigned char i = 0; i < TX_BUFFER_NR; i++) {
            // While descriptors have TOK status, release and advance tail
            unsigned int status = CPU::in32(_io_port + TRSTATUS + _tx_tail * 4);
            if (status & STATUS_TOK) {
                _tx_buffer[_tx_tail]->unlock();

                // Clear TOK status
                CPU::out32(_io_port + TRSTATUS + _tx_tail * 4, status & ~STATUS_TOK);

                db<RTL8139>(WRN) << "\tTOK unlocked " << _tx_tail << endl;
                _tx_tail = (_tx_tail + 1) % TX_BUFFER_NR;
                transmitted = true;
            } else break;
        }

        if (transmitted && _waiting_to_send != nullptr) {
            _waiting_to_send->resume();
            _waiting_to_send = nullptr;
        }
    }

    if (status & ROK) {
        // NIC received frame(s)
        db<RTL8139>(WRN) << "ROK" << endl;

        while ((CPU::in16(_io_port + CBR) < _rx_read) || 
            ((_rx_read + sizeof(Frame)) < CPU::in16(_io_port + CBR))
            )
            receive();

        NetService::resume();
    }

    // Acknowledge interrupt
    CPU::out16(_io_port + ISR, status);
}


void RTL8139::int_handler(const IC::Interrupt_Id & interrupt)
{
    RTL8139 * dev = get_by_interrupt(interrupt);

    db<RTL8139>(WRN) << "RTL8139::int_handler(int=" << interrupt << ",dev=" << dev << ")" << endl;

    if(!dev)
        db<RTL8139>(WRN) << "RTL8139::int_handler: handler not assigned!" << endl;
    else
        dev->handle_int();
}

__END_SYS
