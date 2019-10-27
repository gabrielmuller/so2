#include <netservice.h>
#include <time.h>

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
    
    FrameHeader header = buf->frame()->data<FrameHeader>()[0];
    PortState * state = port_state(header.port);

    if (header.flag == FrameHeader::ACK) {
        db<NetService>(WRN) << "ACK received" << header.port << endl;
        state->resume();
        return;
    }

    db<NetService>(WRN) << "Insert buffer from port " << header.port << endl;

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
    RxBuffer * buf;
    Ethernet::Frame * frame;
    PortState * state = port_state(port);
    while (true) {
        buf = remove_buffer(port);
        frame = buf->frame();
        FrameHeader& header = buf->frame()->data<FrameHeader>()[0];

        if (header.id < state->recv_id) {
            db<NetService>(WRN) << "Duplicate packet id " << header.id << endl;
        } else if (header.id > state->recv_id) {
            // Skips packets, reordering packets is out of scope for this project
            db<NetService>(WRN) << "Out of order packet id " << header.id << " " << state->recv_id << endl;
        } else break;
    }

    *src = frame->src();
    *prot = frame->prot();
    memcpy(data, frame->data<char>() + HEADER_SIZE, size);

    db<NetService>(WRN) << "Receiving package id " << state->recv_id << endl;

    // Send ACK
    {
        db<NetService>(WRN) << "Send ACK through port " << port << endl;
        FrameHeader header = FrameHeader{port, state->recv_id, FrameHeader::ACK};
        nic->send(*src, *prot, &header, HEADER_SIZE);
    }

    state->recv_id++;
    return size;
}

void NetService::timeout() 
{
    db<NetService>(WRN) << "Timeout\n" << endl;
}

int NetService::send(const Address & dst, const Protocol & prot, 
        unsigned short port, const void * data, unsigned int size)
{   
    PortState * state = port_state(port);

    db<NetService>(WRN) << "Send through port " << port << endl;
    
    FrameHeader header = FrameHeader{port, state->send_id, FrameHeader::NONE};
    char buffer[size + HEADER_SIZE];
    memcpy(buffer, &header, HEADER_SIZE);
    memcpy(buffer + HEADER_SIZE, data, size);
    
    db<NetService>(WRN) << "Data " << buffer + HEADER_SIZE << endl;
    
    state->send_id++;

    Function_Handler fh(&timeout);

    int ret = nic->send(dst, prot, buffer, size + HEADER_SIZE);
    Alarm(1000000, &fh);
    state->suspend();
    return ret;
}

__END_SYS
