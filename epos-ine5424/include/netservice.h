#include <process.h>
#include <network/ethernet.h>
#include <machine/nic.h>
#include <utility/hash.h>
#include <time.h>
#include <synchronizer.h>

__BEGIN_SYS

typedef NIC<Ethernet>::Address Address;
typedef NIC<Ethernet>::Protocol Protocol;
typedef Ethernet::Buffer RxBuffer;
typedef RxBuffer::List RxQueue;

class Timeout_Handler: public Handler
{
public:
    Timeout_Handler(void (* h)(unsigned short, unsigned short), unsigned short port, unsigned short id)
            : _handler(h), port(port), id(id) {}
    ~Timeout_Handler() {}
    void operator()() { _handler(port, id); }
private:
    void (* _handler)(unsigned short, unsigned short);
    const unsigned short port;
    const unsigned short id;
};

namespace NetService {
    struct PortState {
        RxQueue queue;              // Received buffers queue
        short retx_left;            // How many retransmissions left
        Semaphore rx_sem;

        // This should be a Mutex, but it's causing linker errors for some reason
        Semaphore tx_mut; 

        unsigned int send_id;     // ID of next frame to send
        unsigned int recv_id;     // ID of next frame to receive
        unsigned int alarm_id;    // ID of next frame a timeout will process
        unsigned int ack_id;      // ID of next frame to be acknowledged

        PortState();
    };

    struct FrameHeader {
        unsigned short port;
        unsigned int id;
        enum flag : unsigned char { NONE = 0x00, ACK = 0x06 } flag;
    };

    const unsigned short HEADER_SIZE = sizeof(NetService::FrameHeader);
    PortState * port_state(unsigned short port);
    void insert_buffer(RxBuffer * buf);
    RxBuffer * remove_buffer(unsigned int port);
    void timeout(unsigned short, unsigned short);

    extern Hash<PortState, 10> ports;
    extern NIC<Ethernet> * nic;

    int receive(Address * src, Protocol * prot, unsigned short port, void * data, unsigned int size);
    int send(const Address & dst, const Protocol & prot, const unsigned short port, const void * data, unsigned int size);
};

__END_SYS
