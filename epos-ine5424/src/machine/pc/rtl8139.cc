// EPOS PC AMD PCNet II (Am79C970A) Ethernet NIC Mediator Implementation

#include <machine/machine.h>
#include <machine/pc/rtl8139.h>
#include <system.h>
#include <time.h>

__BEGIN_SYS

// Class attributes
RTL8139::Device RTL8139::_device;


// Methods
RTL8139::~RTL8139()
{
    db<RTL8139>(TRC) << "~RTL8139()" << endl;
}


int RTL8139::send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
{
    new (_tx_base[_tx_head]) Frame(_address, dst, prot, data, size);
    unsigned short status = sizeof(Frame) & 0xfff;

    while (tx_is_using(_tx_head)) {
        // How to remove this busy wait?
    }
    tx_use(_tx_head);

    CPU::out32(_io_port + TRSTART  + _tx_head * 4, (long unsigned int) _tx_base_phy[_tx_head]);
    CPU::out32(_io_port + TRSTATUS + _tx_head * 4, status);

    _tx_head = (_tx_head + 1) % TX_BUFFER_NR;
    db<RTL8139>(WRN) << dst << endl;

    return size;
}


int RTL8139::receive(Address * src, Protocol * prot, void * data, unsigned int size)
{
    return size;
}


// Allocated buffers must be sent or release IN ORDER as assumed by the RTL8139
RTL8139::Buffer * RTL8139::alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload)
{
    db<RTL8139>(TRC) << "RTL8139::alloc(s=" << _address << ",d=" << dst << ",p=" << hex << prot << dec << ",on=" << once << ",al=" << always << ",pl=" << payload << ")" << endl;

    int max_data = MTU - always;

    if((payload + once) / max_data > TX_BUFS) {
        db<RTL8139>(WRN) << "RTL8139::alloc: sizeof(Network::Packet::Data) > sizeof(NIC::Frame::Data) * TX_BUFS!" << endl;
        return 0;
    }

    Buffer::List pool;

    // Calculate how many frames are needed to hold the transport PDU and allocate enough buffers
    for(int size = once + payload; size > 0; size -= max_data) {
        // Wait for the next buffer to become free and seize it
        unsigned int i = _tx_cur;
        for(bool locked = false; !locked; ) {
            for(; _tx_ring[i].status & Tx_Desc::OWN; ++i %= TX_BUFS);
            locked = _tx_buffer[i]->lock();
        }
        _tx_cur = (i + 1) % TX_BUFS;
        Tx_Desc * desc = &_tx_ring[i];
        Buffer * buf = _tx_buffer[i];

        // Initialize the buffer and assemble the Ethernet Frame Header
        new (buf) Buffer(this, (size > max_data) ? MTU : size + always, _address, dst, prot);

        db<RTL8139>(INF) << "RTL8139::alloc:desc[" << i << "]=" << desc << " => " << *desc << endl;
        db<RTL8139>(INF) << "RTL8139::alloc:buf=" << buf << " => " << *buf << endl;

        pool.insert(buf->link());
    }

    return pool.head()->object();
}


int RTL8139::send(Buffer * buf)
{
    return 0;
}


void RTL8139::free(Buffer * buf)
{
    db<RTL8139>(TRC) << "RTL8139::free(buf=" << buf << ")" << endl;

    db<RTL8139>(INF) << "RTL8139::free:buf=" << buf << " => " << *buf << endl;

    for(Buffer::Element * el = buf->link(); el; el = el->next()) {
        buf = el->object();
        Rx_Desc * desc = reinterpret_cast<Rx_Desc *>(buf->back());

        _statistics.rx_packets++;
        _statistics.rx_bytes += buf->size();

        // Release the buffer to the NIC
        desc->size = Reg16(-sizeof(Frame)); // 2's comp.
        desc->status = Rx_Desc::OWN; // Owned by NIC

        // Release the buffer to the OS
        buf->unlock();

        db<RTL8139>(INF) << "RTL8139::free:desc=" << desc << " => " << *desc << endl;
    }
}


void RTL8139::reset()
{

    db<RTL8139>(WRN) << "RTL8139::reset()" << endl;

    // Power on the NIC
    CPU::out8(_io_port + CONFIG_1, POWER_ON);

    db<RTL8139>(WRN) << "DMA Buffer is at phy " << _dma_buf->phy_address() << endl;

    // Reset the device
    s_reset();

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

    const char msg[] = "Frame frame frame hellooooo";
    for (int i = 0; i < 1000; i++) send(0xbabaca + i, 0x0806, msg, sizeof(msg));

    db<RTL8139>(WRN) << "RBSTART is " << CPU::in32(_io_port + RBSTART) << endl;

}

void RTL8139::tx_use(unsigned char index)
{
    _tx_busy |= 1 << index;
}

void RTL8139::tx_release(unsigned char index)
{
    _tx_busy &= ~(1 << index);
}

volatile bool RTL8139::tx_is_using(unsigned char index)
{
    return (_tx_busy & (1 << index)) != 0;
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
        for (unsigned char i = 0; i < TX_BUFFER_NR; i++) {
            // While descriptors have TOK status, release and advance tail
            if (tx_is_using(_tx_tail) &&
                CPU::in32(_io_port + TRSTATUS + _tx_tail * 4) & STATUS_TOK) {
                tx_release(_tx_tail);
                _tx_tail = (_tx_tail + 1) % TX_BUFFER_NR;
            } else break;
        }
    }

    if (status & ROK) {
        // NIC received frame(s)
        db<RTL8139>(WRN) << "ROK" << endl;
        // TODO
    }

    // Acknowledge interrupt
    CPU::out16(_io_port + ISR, status);
}


void RTL8139::int_handler(const IC::Interrupt_Id & interrupt)
{
    RTL8139 * dev = _device;

    db<RTL8139>(WRN) << "RTL8139::int_handler(int=" << interrupt << ",dev=" << dev << ")" << endl;

    if(!dev)
        db<RTL8139>(WRN) << "RTL8139::int_handler: handler not assigned!" << endl;
    else
        dev->handle_int();
}

__END_SYS
