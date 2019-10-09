#ifndef __rtl8139_h
#define __rtl8139_h

#include <architecture.h>
#include <utility/convert.h>
#include <network/ethernet.h>

__BEGIN_SYS

class RTL8139: NIC<Ethernet>
{
    friend class Machine_Common;

private:
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

protected:
    RTL8139(unsigned int unit, IO_Port io_port, IO_Irq irq, DMA_Buffer * dma_buf);

private:
    static void init(unsigned int unit);

    void handle_int();

    struct Device {
        RTL8139 * device;
        unsigned int interrupt;
    };

    static const unsigned int UNITS = Traits<PCNet32>::UNITS;
    static Device _devices[UNITS];
};

// inline RTL8139::Timer::Time_Stamp RTL8139::Timer::Timer::read() { return TSC::time_stamp(); }
// inline void RTL8139::Timer::adjust(const RTL8139::Timer::Offset & o) { TSC::time_stamp(o); }
// inline RTL8139::Timer::Hertz RTL8139::Timer::frequency() { return TSC::frequency(); }
// inline RTL8139::Timer::PPB RTL8139::Timer::accuracy() { return TSC::accuracy(); }
// inline RTL8139::Timer::Time_Stamp RTL8139::Timer::us2count(const RTL8139::Timer::Microsecond & us) { return Convert::us2count<Time_Stamp, Microsecond>(TSC::frequency(), us); }
// inline RTL8139::Timer::Microsecond RTL8139::Timer::count2us(const RTL8139::Timer::Time_Stamp & ts) { return Convert::count2us<Hertz, Time_Stamp, Microsecond>(TSC::frequency(), ts); }

__END_SYS

#endif
