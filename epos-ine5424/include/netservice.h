#include <process.h>
#include <network/ethernet.h>

__BEGIN_SYS

class NetService {
    static bool suspended;
    static Thread * self;
    static Ethernet::Buffer::List received;
public:
    static int start();
    static void suspend();
    static void resume();
    static void insert_buffer(const Ethernet::Buffer * buf);
    static Ethernet::Buffer * remove_buffer();

};

__END_SYS
