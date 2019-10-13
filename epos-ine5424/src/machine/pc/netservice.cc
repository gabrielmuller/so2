#include <netservice.h>

__BEGIN_SYS

Thread * NetService::self;
bool NetService::suspended;
NIC<Ethernet> * NetService::nic;
Ethernet::Buffer::List NetService::received;

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

void NetService::insert_buffer(const Ethernet::Buffer * buf) {
    received.insert(new Ethernet::Buffer::List::Element(buf));
    db<NetService>(WRN) << "queue size " << received.size() << endl;
}

Ethernet::Buffer * NetService::remove_buffer() {
    db<NetService>(WRN) << "removed queue size " << received.size() << endl;
    return received.remove()->object();
}

int NetService::receive(Address * src, Protocol * prot, void * data, unsigned int size) 
{
    if (received.empty())
        suspend();
    Ethernet::Buffer * buf = remove_buffer();
    Ethernet::Frame * frame = buf->frame();
    *src = frame->src();
    *prot = frame->prot();
    memcpy(data, frame->data<void>(), size);
    return size;
}

int NetService::send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
{
    return nic->send(dst, prot, data, size);
}

__END_SYS
