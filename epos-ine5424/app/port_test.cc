// EPOS Port Test Programs

#include <machine/nic.h>
#include <time.h>
#include <netservice.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Port test" << endl;
    Thread * threads[5];
    for (unsigned short i = 0; i < 5; i++) {
        NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
        char data[] = "Port 100# sends message #.";

        NIC<Ethernet>::Address self = nic->address();
        cout << "  MAC: " << self << endl;

        if(self[5] % 2) { // sender
            Delay (5000000);

            for(int port = 0; port < 5; port++) {
                data[8]  = '0' + port;
                data[24] = '0' + i;
                NetService::send(nic->broadcast(), 0x8888, port, data, sizeof(data));
                //Delay(500000);
            }
        } else {
            NIC<Ethernet>::Address src;
            NIC<Ethernet>::Protocol prot;

            for(int port = 0; port < 5; port++) {
                NetService::receive(&src, &prot, port, data, sizeof(data));
                cout << "\n  Recv data: \"" << data << "\"" << endl;
                cout << "  prot: " << hex << prot << endl;
                cout << "  src: " << src << endl;
            }
        }
    }


    cout << "Port test done" << endl;
}
