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
    
    char* portc[4];
    memcpy(portc, buf->frame()->data<void>(), 4);
    unsigned int port = *((unsigned int*)portc);

    db<NetService>(WRN) << "Insert buffer from port " << *((unsigned int*)portc) << endl;

    if (!ports.search_key(port)) {
        db<NetService>(WRN) << "Creating new port " << port << endl;
        ports.insert(new Hash<Ethernet::Buffer::List, 10>::Element(new Ethernet::Buffer::List(), port));
    }

    Ethernet::Buffer::List* portReceived = ports.search_key(port)->object();
    portReceived->insert(new Ethernet::Buffer::List::Element(buf));
    db<NetService>(WRN) << "Port " << port << " queue size " << portReceived->size() << endl;
}

Ethernet::Buffer * NetService::remove_buffer(unsigned int port) {
    if (!ports.search_key(port)) suspend(); 

    Ethernet::Buffer::List* portReceived = ports.search_key(port)->object();

    if (portReceived->empty()) suspend(); 

    db<NetService>(WRN) << "Port " << port << " removed queue size " << portReceived->size() << endl;
    
    return portReceived->remove()->object();
}

int NetService::receive(Address * src, Protocol * prot, unsigned int port, void * data, unsigned int size) 
{
    Ethernet::Buffer * buf = remove_buffer(port);
    // if (buf == nullptr) {
    //     suspend();
    //     *buf = remove_buffer(port);
    // }
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

    db<NetService>(WRN) << "Send through port " << port << endl;
    
    char buffer[4 + size];
    memcpy(buffer, &port, 4);
    memcpy(buffer + 4, data, size);
    db<NetService>(WRN) << "Data " << *buffer << endl;
    return nic->send(dst, prot, buffer, size);
}

__END_SYS
