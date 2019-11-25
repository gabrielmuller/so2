#include <machine/nic.h>
#include <time.h>
#include <netinterface.h>

using namespace EPOS;

OStream cout;

int main()
{

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    NIC<Ethernet>::Address self = nic->address();
    char msg[100];
    MockGPS gps(UART(1, 115200, 8, 0, 1));

    if (self[5] % 2 == 1) return 0; // Ignore second QEMU

    cout <<  "Serial NMEA test" << endl;
    NetService::sync(gps, true);
    Clock::Date d = RTC::date();
    cout << d.year() << "/" << d.month() << "/" << d.day() << " " << d.hour() << ":" << d.minute() << ":" << d.second() << endl;
    cout << "XYZ = ("
         << (int) NetService::device().x << ", "
         << (int) NetService::device().y << ", "
         << (int) NetService::device().z << ")\n"; 
}
