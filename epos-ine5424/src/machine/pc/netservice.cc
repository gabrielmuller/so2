#include <netservice.h>
#include <network/ethernet.h>

__BEGIN_SYS

Thread * NetService::self;
bool NetService::suspended;
Ethernet::Buffer::List NetService::received;

int NetService::start() 
{
    suspended = false;
    self = Thread::self();
    suspend();

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



__END_SYS
