#include <netservice.h>

__BEGIN_SYS

Thread * NetService::self;
bool NetService::suspended;

int NetService::start() 
{
    suspended = false;
    self = Thread::self();
    db<NetService>(WRN) << self << " thread before suspending" << endl;
    self->suspend();
    db<NetService>(WRN) << self << " after suspending" << endl;

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
        self->resume();
        suspended = false;
    }
}

__END_SYS
