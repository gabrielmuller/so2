#include <process.h>
#include <network/ethernet.h>
#include <machine/nic.h>
#include <utility/hash.h>
#include <time.h>
#include <synchronizer.h>

__BEGIN_SYS

namespace NetService {
    typedef NIC<Ethernet>::Address Address;
    typedef NIC<Ethernet>::Protocol Protocol;

    struct SpaceTime {
        Clock::Date date;
        Clock::Microsecond ms;
        float x, y, z;
    };

    SpaceTime device();
    int receive(Address * src, Protocol * prot, unsigned short port, void * data, unsigned int size);
    int send(const Address & dst, const Protocol & prot, const unsigned short port, const void * data, unsigned int size);
    void sync(bool);
};

__END_SYS
