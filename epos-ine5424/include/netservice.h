#include <process.h>
#include <network/ethernet.h>
#include <machine/nic.h>
#include <utility/hash.h>

__BEGIN_SYS

typedef NIC<Ethernet>::Address Address;
typedef NIC<Ethernet>::Protocol Protocol;
typedef Ethernet::Buffer RxBuffer;
typedef RxBuffer::List RxQueue;

static const unsigned short PORT = sizeof(unsigned short);
static const unsigned short PCKG_ID = sizeof(unsigned short);

class NetService {
    struct PortState {
        RxQueue queue;    // Received buffers queue
        short retx_left;  // How many retransmissions left
        Thread * waiting; // Used to block receiving thread when queue is empty
        short pckg_id;

        PortState() : retx_left(3), waiting(nullptr), pckg_id(0) {}

        void suspend() {
            waiting = Thread::self();
            waiting->suspend();
        }

        void resume() {
            if (waiting != nullptr) {
                Thread * tmp = waiting;
                waiting = nullptr;
                tmp->resume();
            }
        }

    };

    static PortState * port_state(unsigned short port);
    static void insert_buffer(RxBuffer * buf);
    static RxBuffer * remove_buffer(unsigned int port);
    static Hash<PortState, 10> ports;
    static NIC<Ethernet> * nic;
    friend class RTL8139;
public:
    static int receive(Address * src, Protocol * prot, unsigned short port, void * data, unsigned int size);
    static int send(const Address & dst, const Protocol & prot, const unsigned short port, const void * data, unsigned int size);
};

__END_SYS
