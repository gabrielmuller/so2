#include <process.h>

__BEGIN_SYS

class NetService {
    static Thread * self;
    static bool suspended;
public:
    static int start();
    static void suspend();
    static void resume();

};

__END_SYS
