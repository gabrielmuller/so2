#include <netservice.h>

__BEGIN_SYS


NIC<Ethernet> * NetService::nic;
Hash<NetService::PortState, 10> NetService::ports;

NetService::PortState::PortState() : retx_left(Traits<Network>::RETRIES), rx_sem(0), tx_mut(0),
    send_id(0), recv_id(0), ack_id(0) {}

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

    state->queue.insert(new RxQueue::Element(buf));
    state->rx_sem.v();
    db<RTL8139>(WRN) << "Port " << header.port << " inserted, queue size " << state->queue.size() << endl;
}

RxBuffer * NetService::remove_buffer(unsigned int port) {
    PortState * state = port_state(port);
    state->rx_sem.p();
    db<RTL8139>(WRN) << "Port " << port << " removed, queue size " << state->queue.size() << endl;
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

void NetService::timeout(unsigned short port, unsigned short id, const Address & dst, const Protocol & prot, const void * data, unsigned int size) 
{
    db<RTL8139>(WRN) << "Timeout port " << port << " id " << id << endl;
    PortState * state = port_state(port);

    if (id < state->ack_id) {
        db<RTL8139>(WRN) << "Timeout ack received port " << port << " id " << id << endl;
        return;
    }

    if (state->retx_left <= 0) {
        db<RTL8139>(WRN) << "Out of retransmissions port " << port << " id " << id << endl;
        state->tx_mut.v();
        return;
    }

    db<RTL8139>(WRN) << "Retransmit port " << port << " id " << id << endl;
    state->retx_left--;

    nic->send(dst, prot, data, size);

    /* Set alarm again */
    Timeout_Handler * th = new Timeout_Handler(&timeout, port, id, dst, prot, data, size);
    new Alarm(Traits<Network>::TIMEOUT * 1000000, th);
}

int NetService::send(const Address & dst, const Protocol & prot, 
        unsigned short port, const void * data, unsigned int size)
{   
    PortState * state = port_state(port);

    db<RTL8139>(TRC) << "Send through port " << port << endl;
    
    FrameHeader header = FrameHeader{port, state->send_id, FrameHeader::NONE};
    char buffer[size + HEADER_SIZE];
    memcpy(buffer, &header, HEADER_SIZE);
    memcpy(buffer + HEADER_SIZE, data, size);
    
    db<RTL8139>(TRC) << "Sending data: \"" << buffer + HEADER_SIZE << "\"" << endl;
    
    Timeout_Handler th(&timeout, port, state->send_id, dst, prot, buffer, size);
    Alarm timeout_alarm(Traits<Network>::TIMEOUT * 1000000, &th);

    /* Simulating a send failure */
    if (Traits<Network>::SEND_FAULT && port == 0) {
        /* Port 0 fails to send. */
        db<RTL8139>(WRN) << "(Fault injection) Failed to send frame id " 
        << state->send_id << " port " << port << endl;
    } else if (Traits<Network>::SEND_FAULT && port == 1) {
        /* Port 1 is slow (takes 1.5x timeout time), but sends eventually. */
        Delay(Traits<Network>::TIMEOUT * 1500000);
        nic->send(dst, prot, buffer, size + HEADER_SIZE);
    } else {
        /* Ports 2 to 4 work properly. */
        nic->send(dst, prot, buffer, size + HEADER_SIZE);
    }

    state->send_id++;
    state->tx_mut.p();

    if (state->retx_left > 0)
        return size; // Awaken by ACK (success)
    else {
        state->retx_left = Traits<Network>::RETRIES;
        return 0;    // Awaken by max retries (failure)
    }
}

__END_SYS
