#include <process.h>
#include <network/ethernet.h>
#include <machine/nic.h>
#include <utility/hash.h>

__BEGIN_SYS

typedef NIC<Ethernet>::Address Address;
typedef NIC<Ethernet>::Protocol Protocol;
typedef Ethernet::Buffer RxBuffer;
typedef RxBuffer::List RxQueue;

class NetService {
    struct PortState {
        RxQueue queue;              // Received buffers queue
        short retx_left;            // How many retransmissions left
        Thread * waiting;           // Used to block receiving thread when queue is empty

        unsigned int send_id;     // ID of next frame to send
        unsigned int recv_id;     // ID of next frame to receive
        unsigned int alarm_id;    // ID of next frame a timeout will process
        unsigned int ack_id;      // ID of next frame to be acknowledged

        

        PortState() : retx_left(3), waiting(nullptr), 
                send_id(0), recv_id(0), alarm_id(0), ack_id(0) {}

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

    struct FrameHeader {
        unsigned short port;
        unsigned int id;
        enum flag : unsigned char { NONE = 0x00, ACK = 0x06 } flag;
    };

    static const unsigned short HEADER_SIZE = sizeof(NetService::FrameHeader);
    static PortState * port_state(unsigned short port);
    static void insert_buffer(RxBuffer * buf);
    static RxBuffer * remove_buffer(unsigned int port);
    static Hash<PortState, 10> ports;
    static NIC<Ethernet> * nic;
    static void timeout();
    friend class RTL8139;
public:

    static int receive(Address * src, Protocol * prot, unsigned short port, void * data, unsigned int size);
    static int send(const Address & dst, const Protocol & prot, const unsigned short port, const void * data, unsigned int size);
};

__END_SYS
