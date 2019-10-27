// EPOS Port Test Programs

#include <machine/nic.h>
#include <time.h>
#include <netservice.h>

using namespace EPOS;

OStream cout;

int talk_to_port(unsigned short port) {
    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    char data[100];

    NIC<Ethernet>::Address self = nic->address();
    cout << "  MAC: " << self << endl;

    if(self[5] % 2) { // sender
        Delay (5000000);

        for(int i = 0; i < 5; i++) {
            memset(data, '0' + (i % 10), 100);
            data[100 - 1] = '\n';
            NetService::send(nic->broadcast(), 0x8888, port, data, 100);
        }
    } else {
        NIC<Ethernet>::Address src;
        NIC<Ethernet>::Protocol prot;

        for(int i = 0; i < 5; i++) {
           NetService::receive(&src, &prot, port, data, 100);
           cout << "  Data: " << data << endl;
           cout << "  prot: " << hex << prot << endl;
           cout << "  src: " << src << endl;
        }
    }
}

int main()
{
    cout << "Port Test" << endl;
    for (unsigned short i = 0; i < 5; i++)
        (new Thread(&talk_to_port, (unsigned short) (1000 + i)))->join();
}
