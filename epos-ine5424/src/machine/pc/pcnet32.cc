// EPOS PC AMD PCNet II (Am79C970A) Ethernet NIC Mediator Implementation

#include <machine/machine.h>
#include <machine/pc/pcnet32.h>
#include <system.h>
#include <time.h>

__BEGIN_SYS

// Class attributes
PCNet32::Device PCNet32::_devices[UNITS];


// Methods
PCNet32::~PCNet32()
{
    db<PCNet32>(TRC) << "~PCNet32(unit=" << _unit << ")" << endl;
}


int PCNet32::send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
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
    db<PCNet32>(WRN) << dst << endl;

    return size;
}


int PCNet32::receive(Address * src, Protocol * prot, void * data, unsigned int size)
{
    return size;
    db<PCNet32>(TRC) << "PCNet32::receive(s=" << *src << ",p=" << hex << *prot << dec << ",d=" << data << ",s=" << size << ") => " << endl;

    // Wait for a received frame and seize it
    unsigned int i = _rx_cur;
    for(bool locked = false; !locked; ) {
        for(; _rx_ring[i].status & Rx_Desc::OWN; ++i %= RX_BUFS);
        locked = _rx_buffer[i]->lock();
    }
    _rx_cur = (i + 1) % RX_BUFS;
    Buffer * buf = _rx_buffer[i];
    Rx_Desc * desc = &_rx_ring[i];

    // Disassemble the Ethernet frame
    Frame * frame = buf->frame();
    *src = frame->src();
    *prot = frame->prot();

    // For the upper layers, size will represent the size of frame->data<T>()
    buf->size((desc->misc & 0x00000fff) - sizeof(Header) - sizeof(CRC));

    // Copy the data
    memcpy(data, frame->data<void>(), (buf->size() > size) ? size : buf->size());

    // Release the buffer to the NIC
    desc->status = Rx_Desc::OWN;

    _statistics.rx_packets++;
    _statistics.rx_bytes += buf->size();

    db<PCNet32>(INF) << "PCNet32::receive:desc[" << i << "]=" << desc << " => " << *desc << endl;

    int tmp = buf->size();

    buf->unlock();

    return tmp;
}


// Allocated buffers must be sent or release IN ORDER as assumed by the PCNet32
PCNet32::Buffer * PCNet32::alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload)
{
    db<PCNet32>(TRC) << "PCNet32::alloc(s=" << _address << ",d=" << dst << ",p=" << hex << prot << dec << ",on=" << once << ",al=" << always << ",pl=" << payload << ")" << endl;

    int max_data = MTU - always;

    if((payload + once) / max_data > TX_BUFS) {
        db<PCNet32>(WRN) << "PCNet32::alloc: sizeof(Network::Packet::Data) > sizeof(NIC::Frame::Data) * TX_BUFS!" << endl;
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

        db<PCNet32>(INF) << "PCNet32::alloc:desc[" << i << "]=" << desc << " => " << *desc << endl;
        db<PCNet32>(INF) << "PCNet32::alloc:buf=" << buf << " => " << *buf << endl;

        pool.insert(buf->link());
    }

    return pool.head()->object();
}


int PCNet32::send(Buffer * buf)
{
    return 0;
}


void PCNet32::free(Buffer * buf)
{
    db<PCNet32>(TRC) << "PCNet32::free(buf=" << buf << ")" << endl;

    db<PCNet32>(INF) << "PCNet32::free:buf=" << buf << " => " << *buf << endl;

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

        db<PCNet32>(INF) << "PCNet32::free:desc=" << desc << " => " << *desc << endl;
    }
}


void PCNet32::reset()
{

    db<PCNet32>(WRN) << "RTL8139::reset()" << endl;

    // Power on the NIC
    CPU::out8(_io_port + CONFIG_1, POWER_ON);

    db<PCNet32>(WRN) << "DMA Buffer is at phy " << _dma_buf->phy_address() << endl;

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

    db<PCNet32>(WRN) << "RBSTART is " << CPU::in32(_io_port + RBSTART) << endl;

}

void PCNet32::tx_use(unsigned char index)
{
    _tx_busy |= 1 << index;
}

void PCNet32::tx_release(unsigned char index)
{
    _tx_busy &= ~(1 << index);
}

volatile bool PCNet32::tx_is_using(unsigned char index)
{
    return (_tx_busy & (1 << index)) != 0;
}

void PCNet32::handle_int()
{
    /* An interrupt usually means a single frame was transmitted or received,
     * but in certain cases it might handle several frames or none at all.
     */
    unsigned short status = CPU::in16(_io_port + ISR);
    db<PCNet32>(WRN) << "INTERRUPT STATUS " << status << endl;

    if (status & TOK) {
        // Transmit completed successfully, release descriptor
        db<PCNet32>(WRN) << "TOK" << endl;
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
        db<PCNet32>(WRN) << "ROK" << endl;
        // TODO
    }

    // Acknowledge interrupt
    CPU::out16(_io_port + ISR, status);
}


void PCNet32::int_handler(const IC::Interrupt_Id & interrupt)
{
    PCNet32 * dev = get_by_interrupt(interrupt);

    db<PCNet32>(WRN) << "PCNet32::int_handler(int=" << interrupt << ",dev=" << dev << ")" << endl;

    if(!dev)
        db<PCNet32>(WRN) << "PCNet32::int_handler: handler not assigned!" << endl;
    else
        dev->handle_int();
}

__END_SYS
