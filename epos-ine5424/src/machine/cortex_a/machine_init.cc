// EPOS Cortex-A Mediator Initialization

#include <machine.h>

__BEGIN_SYS

// TODO: refactor Realview_PBX::pre_init() to move Cortex-A commonalities here
void Machine::pre_init(System_Info * si)
{
    Machine_Model::pre_init();

    if(Machine::cpu_id() == 0) {
        Display::init();

        if(Traits<IC>::enabled) {
            IC::init();

            // Wake up remaining CPUs
            si->bm.n_cpus = Traits<Build>::CPUS;
            if(Traits<System>::multicore)
                smp_init(si->bm.n_cpus);
        }
    } else
        if(Traits<IC>::enabled)
            IC::int_id(); //clear the wake up interrupt
}

void Machine::init()
{
    db<Init, Machine>(TRC) << "Machine::init()" << endl;

    Machine_Model::init();

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
