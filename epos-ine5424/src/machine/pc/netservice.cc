#include <netservice.h>

__BEGIN_SYS

NIC<Ethernet> * NetService::nic;
Hash<NetService::PortState, 10> NetService::ports;

NetService::PortState * NetService::port_state(unsigned short port) {
    if (!ports.search_key(port)) {
        db<NetService>(WRN) << "Creating new port " << port << endl;
        ports.insert(new Hash<PortState, 10>::Element(new PortState(), port));
    }
    return ports.search_key(port)->object();
}
    
void NetService::insert_buffer(RxBuffer * buf) {
    const Ethernet::Frame * frame = buf->frame();

    // Discard other protocols
    if (frame->prot() != 0x8888) return;
    
    unsigned short port = buf->frame()->data<unsigned short>()[0];

    db<NetService>(WRN) << "Insert buffer from port " << port << endl;

    PortState * state = port_state(port);
    state->queue.insert(new RxQueue::Element(buf));
    state->resume();
    db<NetService>(WRN) << "Port " << port << " queue size " << state->queue.size() << endl;
}

RxBuffer * NetService::remove_buffer(unsigned int port) {
    PortState * state = port_state(port);

    if (state->queue.empty()) 
        state->suspend();

    db<NetService>(WRN) << "Port " << port << " removed queue size " << state->queue.size() << endl;
    
    return state->queue.remove()->object();
}

int NetService::receive(Address * src, Protocol * prot, unsigned short port,
        void * data, unsigned int size) 
{
    RxBuffer * buf = remove_buffer(port);
    Ethernet::Frame * frame = buf->frame();
    *src = frame->src();
    *prot = frame->prot();
    memcpy(data, frame->data<char>() + PORT, size);
    return size;
}

int NetService::send(const Address & dst, const Protocol & prot, 
        unsigned short port, const void * data, unsigned int size)
{   
    port_state(port); // Create port state if non existant

    db<NetService>(WRN) << "Send through port " << port << endl;
    
    char buffer[size + PORT];
    memcpy(buffer, &port, PORT);
    memcpy(buffer + PORT, data, size);
    db<NetService>(WRN) << "Data " << buffer + PORT << endl;
    return nic->send(dst, prot, buffer, size + PORT);
}

__END_SYS
