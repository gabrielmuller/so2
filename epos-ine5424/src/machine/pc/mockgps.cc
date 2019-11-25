#include <mockgps.h>

__BEGIN_SYS

UART MockGPS::uart(1, 115200, 8, 0, 1);

unsigned int MockGPS::digit(const char sentence)
{
    return ((unsigned int) sentence) - 0x30;
}

float MockGPS::parse_float(const char * & sentence)
{
    float f;
    for (f = 0; *sentence != '.';)
        f = f * 10 + digit(*sentence++);

    sentence++;

    for (float mul = 0.1; *sentence != ','; mul *= 0.1)
        f += digit(*sentence++) * mul;

    sentence++;
    return f;
}

inline float MockGPS::cos(float x) 
{
    float ret;
    __asm__("fld %1;"
            "fcos;"
            "fstp %0;" : "=g"(ret) : "g"(x)
    );
    return ret;
}

inline float MockGPS::sin(float x) 
{
    float ret;
    __asm__("fld %1;"
            "fsin;"
            "fstp %0;" : "=g"(ret) : "g"(x)
    );
    return ret;
}

void MockGPS::send(const char* message)
{
    for (; *message; message++)
        uart.put(*message);
    uart.put('\0');
}

void MockGPS::receive(char* output)
{
    for (char c; (c = uart.get()); output++ ) {
        *output = c;
        db<MockGPS>(WRN) << "|" << c;
    }
    *(++output) = '\0';
}

int MockGPS::parse_nmea(
        const char * sentence,
        Clock::Date & date, Clock::Microsecond & ms,
        float & latitude, float & longitude,
        bool & north, bool & east)
{
    // $GPGGA,121314.56,4124.8934,N,08151.6849,W
    // ^ <- *sentence

    while (*sentence++ != ',');

    // $GPGGA,121314.56,4124.8934,N,08151.6849,W
    //        ^
    date.hour  (digit(sentence[0]) * 10 + digit(sentence[1]));
    date.minute(digit(sentence[2]) * 10 + digit(sentence[3]));
    sentence += 4;

    const float seconds = parse_float(sentence);
    const unsigned int int_seconds = seconds;

    date.second(int_seconds);
    ms = seconds - int_seconds;

    // $GPGGA,121314.56,4124.8934,N,08151.6849,W
    //               ^

    latitude = parse_float(sentence) * 0.01;

    north = *sentence == 'N';
    if (*sentence != 'N' && *sentence != 'S') return 1;
    sentence += 2;

    // $GPGGA,121314.56,4124.8934,N,08151.6849,W
    //                           ^
    longitude = parse_float(sentence) * 0.01;

    east = *sentence == 'E';
    if (*sentence != 'E' && *sentence != 'W') return 1;
    sentence += 2;

    // ...
    return 0;
}

void MockGPS::to_xyz(
        const float latitude, const float longitude, // Input
        const bool north, const bool east,           // Input
        float & x, float & y, float & z)             // Output
{
    const float EARTH_RADIUS = 6378000;
    const float DEG2RAD = 3.14159265359 / 180;
    const int lat_sign = north ? 1 : -1;
    const int lon_sign = east  ? 1 : -1;
    const float rad_lat = latitude  * DEG2RAD * lat_sign;
    const float rad_lon = longitude * DEG2RAD * lon_sign;

    x = EARTH_RADIUS * cos(rad_lat) * cos(rad_lon);
    y = EARTH_RADIUS * cos(rad_lat) * sin(rad_lon);
    z = EARTH_RADIUS * sin(rad_lat); 
}

__END_SYS
