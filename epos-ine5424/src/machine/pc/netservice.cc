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
}

Ethernet::Buffer * NetService::remove_buffer() {
    return received.remove()->object();
}

int NetService::receive(Address * src, Protocol * prot, void * data, unsigned int size) 
{
    suspend();
    return nic->receive(src, prot, data, size);
}

int NetService::send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
{
    return nic->send(dst, prot, data, size);
}

__END_SYS
