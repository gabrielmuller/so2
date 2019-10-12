#include <process.h>
#include <network/ethernet.h>
#include <machine/nic.h>

__BEGIN_SYS

typedef NIC<Ethernet>::Address Address;
typedef NIC<Ethernet>::Protocol Protocol;

class NetService {
    static bool suspended;
    static Thread * self;
    static Ethernet::Buffer::List received;
    static NIC<Ethernet> * nic;
public:
    static int start();
    static void suspend();
    static void resume();
    static void insert_buffer(const Ethernet::Buffer * buf);
    static Ethernet::Buffer * remove_buffer();
    static int receive(Address * src, Protocol * prot, void * data, unsigned int size);
    static int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size);
};

__END_SYS
