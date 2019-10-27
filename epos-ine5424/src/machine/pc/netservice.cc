#include <netservice.h>

__BEGIN_SYS

static const unsigned short HEADER_SIZE = sizeof(NetService::FrameHeader);

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
    
    FrameHeader header = buf->frame()->data<FrameHeader>()[0];

    db<NetService>(WRN) << "Insert buffer from port " << header.port << endl;

    PortState * state = port_state(header.port);
    state->queue.insert(new RxQueue::Element(buf));
    state->resume();
    db<NetService>(WRN) << "Port " << header.port << " queue size " << state->queue.size() << endl;
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
    FrameHeader header = buf->frame()->data<FrameHeader>()[0];
    memcpy(data, frame->data<char>() + HEADER_SIZE, size);
    // TODO: send ack
    db<NetService>(WRN) << "Receiving package id " << header.id << endl;
    return size;
}

int NetService::send(const Address & dst, const Protocol & prot, 
        unsigned short port, const void * data, unsigned int size)
{   
    PortState * state = port_state(port); // Create port state if non existant

    db<NetService>(WRN) << "Send through port " << port << endl;
    
    FrameHeader header = FrameHeader{port, state->frame_id};
    char buffer[size + HEADER_SIZE];
    memcpy(buffer, &header, HEADER_SIZE);
    memcpy(buffer + HEADER_SIZE, data, size);
    
    db<NetService>(WRN) << "Data " << buffer + HEADER_SIZE << endl;
    
    state->frame_id++;
    return nic->send(dst, prot, buffer, size + HEADER_SIZE);
}

__END_SYS
