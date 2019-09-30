// EPOS Cortex-A Gigabit Ethernet Controller NIC Mediator Declarations

#ifndef __gem_h
#define __gem_h

#include <network/ethernet.h>

__BEGIN_SYS

class GEM: public NIC<Ethernet> //, private Engine
{
    friend class Machine_Common;

protected:
    GEM(){}

public:
    ~GEM() {}

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size) { return 0; }
    int receive(Address * src, Protocol * prot, void * data, unsigned int size) { return 0; }


    Buffer * alloc(const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload);
    void free(Buffer * buf);
    int send(Buffer * buf);

    const Address & address() { Address * a = new (SYSTEM) Address; return *a; }
    void address(const Address & address) {}

    const Statistics & statistics() { Statistics * s = new (SYSTEM) Statistics; return *s; }

    void reset() {}

    static GEM * get(unsigned int unit = 0) { GEM * g = new (SYSTEM) GEM; return g; }

private:
    static void init(unsigned int unit);
};

__END_SYS

#endif
