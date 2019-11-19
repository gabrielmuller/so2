#include <utility/ostream.h>
#include <machine/uart.h>

__BEGIN_SYS

/* Reads fake GPS information from the COM2 serial port. */
class MockGPS 
{
private:
    static UART uart;

public:
    static void send(const char* message) {
        for (unsigned int i = 0; message[i] != 0x00; i++) {
            uart.put(message[i]);
        }
    }

    static void receive(char* output, unsigned int size = 100) {
        for (unsigned int i = 0; i < size; i++) {
            const char c = uart.get();
            output[i] = c;
            if (c == 0x00) 
                break;
        }
    }
};

UART MockGPS::uart(1, 115200, 8, 0, 1);

__END_SYS
