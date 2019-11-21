#include <utility/ostream.h>
#include <machine/uart.h>
#include <time.h>

__BEGIN_SYS

OStream cout;
/* Reads fake GPS information from the COM2 serial port. */
class MockGPS 
{
private:
    static UART uart;

    static unsigned int digit(const char sentence)
    {
        return ((unsigned int) sentence) - 0x30;
    }

    static float parse_float(const char * & sentence)
    {
        float f;
        for (f = 0; *sentence != '.';)
            f = f * 10 + digit(*sentence++);

        sentence++;

        // $GPGGA,121314,4124.8934,N,08151.6849,W
        //                    ^

        for (float mul = 0.1; *sentence != ','; mul *= 0.1)
            f += digit(*sentence++) * mul;

        sentence++;
        return f;
    }

public:
    static void send(const char* message)
    {
        for (; *message; message++)
            uart.put(*message);
        uart.put('\0');
    }

    static void receive(char* output)
    {
        for (char c; (c = uart.get()); output++ )
            *output = c;
        *(++output) = '\0';
    }

    static int parse_nmea(const char * sentence, Clock::Date & date,
            float & latitude, float & longitude, bool & north, bool & east)
    {
        // $GPGGA,121314,4124.8934,N,08151.6849,W
        // ^ <- *sentence

        while (*sentence++ != ',');

        // $GPGGA,121314,4124.8934,N,08151.6849,W
        //        ^
        date.hour  (digit(*sentence++) * 10 + digit(*sentence++));
        date.minute(digit(*sentence++) * 10 + digit(*sentence++));
        date.second(digit(*sentence++) * 10 + digit(*sentence++));

        if (*sentence++ != ',') return 1;
        // $GPGGA,121314,4124.8934,N,08151.6849,W
        //               ^

        latitude = parse_float(sentence);

        north = *sentence == 'N';
        if (*sentence != 'N' && *sentence != 'S') return 1;
        sentence += 2;

        // $GPGGA,121314,4124.8934,N,08151.6849,W
        //                           ^
        longitude = parse_float(sentence);

        east = *sentence == 'E';
        if (*sentence != 'E' && *sentence != 'W') return 1;
        sentence += 2;

        // ...
        return 0;
    }
};

UART MockGPS::uart(1, 115200, 8, 0, 1);

__END_SYS
