#include <netservice.h>

__BEGIN_SYS


NIC<Ethernet> * NetService::nic;
Hash<NetService::PortState, 10> NetService::ports;

NetService::PortState::PortState() : retx_left(3), rx_sem(0), tx_mut(0),
                send_id(0), recv_id(0), alarm_id(0), ack_id(0) {}

NetService::PortState * NetService::port_state(unsigned short port) {
    if (!ports.search_key(port)) {
        db<RTL8139>(WRN) << "Creating new port " << port << endl;
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
        if (header.id == state->ack_id) {
            db<RTL8139>(WRN) << "ACK received for port " << header.port << endl;
            state->ack_id++;
            state->tx_mut.v();
        } else if (header.id < state->ack_id) {
            db<RTL8139>(WRN) << "Late ACK from failed send at port " << header.port << endl;
        } else
            db<RTL8139>(WRN) << "ACK from the future at port " << header.port << endl;


        return;
    }

    db<RTL8139>(WRN) << "Insert buffer from port " << header.port << endl;

    state->queue.insert(new RxQueue::Element(buf));
    state->rx_sem.v();
    db<RTL8139>(WRN) << "Port " << header.port << " queue size " << state->queue.size() << endl;
}

RxBuffer * NetService::remove_buffer(unsigned int port) {
    PortState * state = port_state(port);
    state->rx_sem.p();
    db<RTL8139>(WRN) << "Port " << port << " removed queue size " << state->queue.size() << endl;
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
            db<RTL8139>(WRN) << "Duplicate packet id " << header.id << endl;
        } else if (header.id > state->recv_id) {
            // Skips packets, reordering packets is out of scope for this project
            db<RTL8139>(WRN) << "Out of order packet id " << header.id << " " << state->recv_id << endl;
        } else break;
    }

    *src = frame->src();
    *prot = frame->prot();
    memcpy(data, frame->data<char>() + HEADER_SIZE, size);

    db<RTL8139>(WRN) << "Receiving package id " << state->recv_id << endl;

    // Send ACK
    {
        db<RTL8139>(WRN) << "Send ACK through port " << port << endl;
        FrameHeader header = FrameHeader{port, state->recv_id, FrameHeader::ACK};
        nic->send(*src, *prot, &header, HEADER_SIZE);
    }

    state->recv_id++;
    return size;
}

void NetService::timeout(unsigned short port, unsigned short id) 
{
    db<RTL8139>(WRN) << "Timeout port " << port << " id " << id << endl;
}

int NetService::send(const Address & dst, const Protocol & prot, 
        unsigned short port, const void * data, unsigned int size)
{   
    PortState * state = port_state(port);

    db<RTL8139>(WRN) << "Send through port " << port << endl;
    
    FrameHeader header = FrameHeader{port, state->send_id, FrameHeader::NONE};
    char buffer[size + HEADER_SIZE];
    memcpy(buffer, &header, HEADER_SIZE);
    memcpy(buffer + HEADER_SIZE, data, size);
    
    db<RTL8139>(WRN) << "Sending data: \"" << buffer + HEADER_SIZE << "\"" << endl;
    

    int ret = nic->send(dst, prot, buffer, size + HEADER_SIZE);

    Timeout_Handler th(&timeout, port, state->send_id);
    Alarm timeout_alarm(1000000, &th);

    db<RTL8139>(WRN) << "Create timeout " << &timeout_alarm << endl;

    state->send_id++;

    state->tx_mut.p();
    return ret;
}

__END_SYS
