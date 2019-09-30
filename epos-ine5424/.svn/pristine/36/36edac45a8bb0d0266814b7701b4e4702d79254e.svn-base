// EPOS Cortex-M Mediator Initialization

#include <machine.h>

__BEGIN_SYS

void Machine::pre_init(System_Info * si)
{
    Machine_Model::pre_init();

    Display::init();
}

void Machine::init()
{
    db<Init, Machine>(TRC) << "Machine::init()" << endl;

    Machine_Model::init();

    if(Traits<IC>::enabled)
        IC::init();
    if(Traits<Timer>::enabled)
        Timer::init();
#ifdef __USB_H
    if(Traits<USB>::enabled)
        USB::init();
#endif
    if(Traits<Ethernet>::enabled)
        Initializer<Ethernet>::init();

    if(Traits<IEEE802_15_4>::enabled)
        Initializer<IEEE802_15_4>::init();

    if(Traits<Modem>::enabled)
        Initializer<Modem>::init();
}

__END_SYS
