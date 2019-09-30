// EPOS ARM Cortex-A Mediator Declarations

#ifndef __cortex_a_h
#define __cortex_a_h

#include <architecture.h>
#include <machine/main.h>
#include <machine/rtc.h>
#include __MODEL_H
#include "info.h"
#include "memory_map.h"

__BEGIN_SYS

class Machine: private Machine_Common, private Machine_Model
{
    friend class Init_System;
    friend class First_Object;

private:
    static const bool smp = Traits<System>::multicore;

public:
    Machine() {}

    static void delay(const RTC::Microsecond & time) {
        assert(Traits<TSC>::enabled);
        TSC::Time_Stamp end = TSC::time_stamp() + time * (TSC::frequency() / 1000000);
        while(end > TSC::time_stamp());
    }

    static void panic();
    static void reboot();
    static void poweroff() { reboot(); }

    static unsigned int n_cpus() { return Machine_Model::n_cpus();  }
    static unsigned int cpu_id() { return Machine_Model::cpu_id(); }

    static void smp_init(unsigned int n_cpus) {
        _n_cpus = n_cpus;
        if(smp)
            Machine_Model::smp_init(n_cpus);
    }

    static void smp_barrier(unsigned long n_cpus = _n_cpus) {
        static volatile unsigned long ready[2];
        static volatile unsigned long i;

        if(smp && (n_cpus > 1)) {
            int j = i;

            CPU::finc(ready[j]);

            if(cpu_id() == 0) {
                while(ready[j] < n_cpus); // wait for all CPUs to be ready
                i = !i;                   // toggle ready
                ready[j] = 0;             // signalizes waiting CPUs
            } else
                while(ready[j]);          // wait for CPU[0] signal
        }
    }

    static const UUID & uuid() { return System::info()->bm.uuid; }

private:
    static void pre_init(System_Info * si);
    static void init();

private:
    static volatile unsigned int _n_cpus;
};

__END_SYS

#endif
