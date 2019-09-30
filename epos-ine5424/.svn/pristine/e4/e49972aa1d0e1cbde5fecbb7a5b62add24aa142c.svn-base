// EPOS Cortex-A Mediator Implementation

#include <machine/cortex_a/machine.h>
#include <machine/display.h>

__BEGIN_SYS

// Class attributes
volatile unsigned int Machine::_n_cpus;


// Class methods
void Machine::panic()
{
    CPU::int_disable();
    if(Traits<Display>::enabled)
        Display::puts("PANIC!\n");
    if(Traits<System>::reboot)
        reboot();
    else
        CPU::halt();
}

void Machine::reboot()
{
    db<Machine>(WRN) << "Machine::reboot()" << endl;
    Machine_Model::reboot();
}

__END_SYS
