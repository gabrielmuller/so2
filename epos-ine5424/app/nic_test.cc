// EPOS NIC Test Programs

#include <machine/nic.h>
#include <time.h>
#include <netservice.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "NIC Test" << endl;
    //Thread * a = new Thread(Thread::Configuration(Thread::RUNNING, Thread::HIGH), &NetService::start);

    NetService::start();

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    NIC<Ethernet>::Address src, dst;
    NIC<Ethernet>::Protocol prot;
    char data[nic->mtu()];

    NIC<Ethernet>::Address self = nic->address();
    cout << "  MAC: " << self << endl;

    if(self[5] % 2) { // sender
        Delay (5000000);

        for(int i = 0; i < 100; i++) {
            memset(data, '0' + (i % 10), nic->mtu());
            data[nic->mtu() - 1] = '\n';
            NetService::send(nic->broadcast(), 0x8888, data, nic->mtu());
        }
    } else {
        for(int i = 0; i < 100; i++) {
           NetService::receive(&src, &prot, data, nic->mtu());
           cout << "  Data: " << data;
        }
    }

    NIC<Ethernet>::Statistics stat = nic->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";
}
