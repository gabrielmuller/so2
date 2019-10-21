// EPOS NIC Test Programs

#include <machine/nic.h>
#include <time.h>
#include <netservice.h>

using namespace EPOS;

OStream cout;

int jam_interrupt(RTL8139 * nic, unsigned int interval) {
    /* Disable interrupts periodically in order to force NIC buffer overflows */
    for (unsigned int i = 0; i < 50; i++) {
        nic->disable_int();
        Delay{interval};
        nic->enable_int();
        Delay{interval};
    }
}
    
int main()
{
    cout << "NIC Test" << endl;

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);

    Thread * jammer; 
    NetService::start();

    NIC<Ethernet>::Address src, dst;
    NIC<Ethernet>::Protocol prot;
    char data[nic->mtu()];

    NIC<Ethernet>::Address self = nic->address();
    cout << "  MAC: " << self << endl;

    if(self[5] % 2) { // sender
        Delay (5000000);

        jammer = new Thread(
                Thread::Configuration(Thread::RUNNING, -4), 
                &jam_interrupt,
                (RTL8139 *) nic,
                10000U
        );

        Thread::yield();

        for(int i = 0; i < 100; i++) {
            memset(data, '0' + (i % 10), nic->mtu());
            data[nic->mtu() - 1] = '\n';
            NetService::send(nic->broadcast(), 0x8888, data, nic->mtu());
        }
    } else {
        jammer = new Thread(
                Thread::Configuration(Thread::RUNNING, -4), 
                &jam_interrupt,
                (RTL8139 *) nic,
                6137U
        );

        for(int i = 0; i < 100; i++) {
           NetService::receive(&src, &prot, data, nic->mtu());
           cout << "  Data: " << data << endl;
           cout << "  prot: " << hex << prot << endl;
           cout << "  src: " << src << endl;
        }
    }
}
