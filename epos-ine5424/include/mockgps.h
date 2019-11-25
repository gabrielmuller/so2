#include <utility/ostream.h>
#include <machine/uart.h>
#include <time.h>

__BEGIN_SYS

/* Reads fake GPS information from the COM2 serial port. */
class MockGPS 
{
private:
    UART uart;
    static unsigned int digit(const char sentence);
    static float parse_float(const char * & sentence);
    inline static float cos(float x);
    inline static float sin(float x);

public:
    MockGPS(UART);
    void send(const char* message);
    void receive(char* output);
    static int parse_nmea(
            const char * sentence,
            Clock::Date & date, Clock::Microsecond & ms,
            float & latitude, float & longitude,
            bool & north, bool & east);

    static void to_xyz(
            const float latitude, const float longitude, // Input
            const bool north, const bool east,           // Input
            float & x, float & y, float & z);            // Output
};

__END_SYS
