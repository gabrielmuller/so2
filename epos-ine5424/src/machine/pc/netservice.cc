#include <netservice.h>

__BEGIN_SYS

Thread * NetService::self;
bool NetService::suspended;
NIC<Ethernet> * NetService::nic;
Ethernet::Buffer::List NetService::received;
Hash<Ethernet::Buffer::List, 10> NetService::ports;

int NetService::start() 
{
    suspended = false;
    self = Thread::self();
    nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    return 0;
}

void NetService::suspend() 
{
    if (!suspended) {
        suspended = true;
        self->suspend();
    }
}

void NetService::resume() 
{
    if (suspended) {
        suspended = false;
        self->resume();
    }
}

void NetService::insert_buffer(Ethernet::Buffer * buf) {
    const Ethernet::Frame * frame = buf->frame();

    // Discard other protocols
    if (frame->prot() != 0x8888) return;
    
    // TODO remove port from buf->frame->data
    const unsigned int port = 1;
    if (!ports.search_key(port)) {
        db<NetService>(WRN) << "Creating new port " << port << endl;
        ports.insert(new Hash<Ethernet::Buffer::List, 10>::Element(new Ethernet::Buffer::List(), port));
    }
    received.insert(new Ethernet::Buffer::List::Element(buf));
    db<NetService>(TRC) << "queue size " << received.size() << endl;
}

Ethernet::Buffer * NetService::remove_buffer(unsigned int port) {
    // TODO: remove from port list
    db<NetService>(TRC) << "removed queue size " << received.size() << endl;
    return received.remove()->object();
}

int NetService::receive(Address * src, Protocol * prot, unsigned int port, void * data, unsigned int size) 
{
    if (received.empty()) // TODO: check buffer list
        suspend();
    Ethernet::Buffer * buf = remove_buffer(port);
    Ethernet::Frame * frame = buf->frame();
    *src = frame->src();
    *prot = frame->prot();
    memcpy(data, frame->data<void>(), size);
    return size;
}

int NetService::send(const Address & dst, const Protocol & prot, unsigned int port, 
    const void * data, unsigned int size)
{   
    if (!ports.search_key(port)) {
        db<NetService>(WRN) << "Creating new port " << port << endl;
        ports.insert(new Hash<Ethernet::Buffer::List, 10>::Element(new Ethernet::Buffer::List(), port));
    }
    // TODO: add port to data
    return nic->send(dst, prot, data, size);
}

__END_SYS
