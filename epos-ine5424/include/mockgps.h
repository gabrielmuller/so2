#include <utility/ostream.h>
#include <machine/uart.h>

__BEGIN_SYS

/* Reads fake GPS information from the COM2 serial port. */
class MockGPS 
{
private:
    static UART uart;

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
};

UART MockGPS::uart(1, 115200, 8, 0, 1);

__END_SYS
