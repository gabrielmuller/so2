// EPOS Port Test Programs

#include <machine/nic.h>
#include <time.h>
#include <netservice.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Port Test" << endl;

    NetService::start();

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    NIC<Ethernet>::Address src, dst;
    NIC<Ethernet>::Protocol prot;
    char data[100];

    NIC<Ethernet>::Address self = nic->address();
    cout << "  MAC: " << self << endl;

    if(self[5] % 2) { // sender
        Delay (5000000);

        for(int i = 0; i < 5; i++) {
            memset(data, '0' + (i % 10), 100);
            data[100 - 1] = '\n';
            NetService::send(nic->broadcast(), 0x8888, 15, data, 100);
        }
    } else {
        for(int i = 0; i < 5; i++) {
           NetService::receive(&src, &prot, 15, data, 100);
           cout << "  Data: " << data << endl;
           cout << "  prot: " << hex << prot << endl;
           cout << "  src: " << src << endl;
        }
    }
}
