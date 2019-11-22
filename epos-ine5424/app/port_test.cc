// EPOS Port Test Programs

#include <machine/nic.h>
#include <time.h>
#include <netinterface.h>
#include <mockgps.h>

using namespace EPOS;

OStream cout;
const char p[] = "(APP) ";


int main()
{

    cout << p << "Port test" << endl;
    NetService::sync(true);
    Clock::Date d = RTC::date();
    cout << d.year() << "/" << d.month() << "/" << d.day() << " " << d.hour() << ":" << d.minute() << ":" << d.second() << endl;
    cout << (int) NetService::device().x << " "
         << (int) NetService::device().y << " "
         << (int) NetService::device().z << endl; 

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    NIC<Ethernet>::Address self = nic->address();

    char msg[100];
    if (self[5] % 2 == 0) // Communicates with daemon
        for (unsigned int i = 0; i < 5; i++) {
            MockGPS::receive(msg);
            MockGPS::send("Hewwo!");
            cout << msg << endl;
        }

    for (unsigned short i = 0; i < 5; i++) {
        char data[] = "Port # sends message #.";

        cout << p << "  MAC: " << self << endl;

        if(self[5] % 2) { // sender
            cout << "\n";
            for(unsigned short port = 0; port < 5; port++) {
                data[5]  = '0' + port;
                data[21] = '0' + i;
                cout << p << "| Send data: \"" << data << "\"\n\n";
                int ret = NetService::send(nic->broadcast(), 0x8888, port, data, sizeof(data));
                if (ret == 0) cout << p << "Send fail detected!\n";
                Delay(500000);
            }
        } else {
            Delay (5000000); // Trigger send timeout at start
            NIC<Ethernet>::Address src;
            NIC<Ethernet>::Protocol prot;

            for(int port = 1; port < 5; port++) { // port zero will fail
                Delay(312345);
                NetService::receive(&src, &prot, port, data, sizeof(data));
                cout << p << "| Recv data: \"" << data << "\"\n\n";
                cout << p << "| prot:       " << hex << prot << endl;
                cout << p << "| src:        " << src << endl;
            }
        }
    }


    cout << p << "Port test done" << endl;
}
