#ifndef __rtl8139_h
#define __rtl8139_h

#include <architecture.h>
#include <utility/convert.h>
#include <network/ethernet.h>

__BEGIN_SYS

class RTL8139: public NIC<Ethernet>
{
    friend class Machine_Common;

private:
    // RTL8139 registers
    enum {
        MAC0_5     = 0x00,
        MAR0_7     = 0x08,
        TRSTATUS   = 0x10,
        TRSTART    = 0x20,
        RBSTART    = 0x30,
        CMD        = 0x37,
        CAPR       = 0x38,
        CBR        = 0x3A,
        IMR        = 0x3C,
        ISR        = 0x3E,
        RCR        = 0x44,
        CONFIG_1   = 0x52,
    };

    // RTL8139 values
    enum {
        POWER_ON   = 0x00,
        RESET      = 0x10,
        TOK        = 0x04,
        ROK        = 0x01,
        ACCEPT_ANY = 0x0F,
        TE         = 0x04,
        RE         = 0x08,
        WRAP       = 0x80,
        STATUS_TOK = 0x8000,
    };

    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::IO_Port IO_Port;
    typedef CPU::IO_Irq IO_Irq;
    typedef MMU::DMA_Buffer DMA_Buffer;
    typedef Ethernet::Address MAC_Address;

    // PCI ID
    static const unsigned int PCI_VENDOR_ID = 0x10EC;
    static const unsigned int PCI_DEVICE_ID = 0x8139; // RTL8139
    static const unsigned int PCI_REG_IO = 0;
    static const unsigned int UNITS = Traits<RTL8139>::UNITS;

    // Buffer config
    static const unsigned int RX_NO_WRAP_SIZE = 8192;
    static const unsigned int RX_BUFFER_SIZE = (RX_NO_WRAP_SIZE + 16 + 1522 + 3) & ~3;
    static const unsigned int TX_BUFFER_SIZE = (sizeof(Frame) + 3) & ~3;
    static const unsigned int TX_BUFFER_NR = 4;

    // Size of the DMA Buffer
    static const unsigned int DMA_BUFFER_SIZE = RX_BUFFER_SIZE + TX_BUFFER_SIZE * 4;

public:
    ~RTL8139();

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size);
    void receive();
    int receive(Address * src, Protocol * prot, void * data, unsigned int size) { return size; }

    Buffer * alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload);
    int send(Buffer * buf);
    void free(Buffer * buf);

    const Address & address() { return _address; }
    void address(const Address & address) { _address = address; }

    const Statistics & statistics() { return _statistics; }

    void reset();

    static RTL8139 * get(unsigned int unit = 0) { return _devices[unit].device; }

protected:
    RTL8139(unsigned int unit, IO_Port io_port, IO_Irq irq, DMA_Buffer * dma_buf);

private:
    unsigned int _unit;

    static void init(unsigned int unit);

    void handle_int();

    static void int_handler(const IC::Interrupt_Id & interrupt);

    static RTL8139 * get_by_interrupt(unsigned int interrupt) {
        for(unsigned int i = 0; i < UNITS; i++)
            if(_devices[i].interrupt == interrupt)
                return _devices[i].device;
        db<RTL8139>(WRN) << "RTL8139::get_by_interrupt(" << interrupt << ") => no device bound!" << endl;
        return 0;
    };

    struct Device {
        RTL8139 * device;
        unsigned int interrupt;
    };

    static Device _devices[UNITS];

    Statistics _statistics;
    Address _address;
    IO_Irq _irq;
    DMA_Buffer * _dma_buf;
    IO_Port _io_port;

    char * _rx_buffer;
    Buffer * _tx_buffer[TX_BUFFER_NR];


    volatile unsigned char _tx_tail = 0; // What descriptor is the NIC using?
    unsigned char _tx_head = 0;          // What descriptor is the CPU using?

    char * _tx_base_phy[TX_BUFFER_NR];
    char * _rx_base_phy;
    unsigned int _rx_read = 0;

    Thread * _waiting_to_send;
};

__END_SYS

#endif
