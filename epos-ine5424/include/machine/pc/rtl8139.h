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

public:
    ~RTL8139();

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size);
    int receive(Address * src, Protocol * prot, void * data, unsigned int size);

    Buffer * alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload);
    int send(Buffer * buf);
    void free(Buffer * buf);
    bool drop(unsigned int id);

    const Address & address();
    void address(const Address &);

    unsigned int period();

    const Statistics & statistics();

    void reset();

    static RTL8139 * get(unsigned int unit = 0) { return _device.device; }

protected:
    RTL8139(unsigned int unit, IO_Port io_port, IO_Irq irq, DMA_Buffer * dma_buf);

private:
    static void init(unsigned int unit);

    void handle_int();

    struct Device {
        RTL8139 * device;
        unsigned int interrupt;
    };

    static Device _device;
};

// inline RTL8139::Timer::Time_Stamp RTL8139::Timer::Timer::read() { return TSC::time_stamp(); }
// inline void RTL8139::Timer::adjust(const RTL8139::Timer::Offset & o) { TSC::time_stamp(o); }
// inline RTL8139::Timer::Hertz RTL8139::Timer::frequency() { return TSC::frequency(); }
// inline RTL8139::Timer::PPB RTL8139::Timer::accuracy() { return TSC::accuracy(); }
// inline RTL8139::Timer::Time_Stamp RTL8139::Timer::us2count(const RTL8139::Timer::Microsecond & us) { return Convert::us2count<Time_Stamp, Microsecond>(TSC::frequency(), us); }
// inline RTL8139::Timer::Microsecond RTL8139::Timer::count2us(const RTL8139::Timer::Time_Stamp & ts) { return Convert::count2us<Hertz, Time_Stamp, Microsecond>(TSC::frequency(), ts); }

__END_SYS

#endif
