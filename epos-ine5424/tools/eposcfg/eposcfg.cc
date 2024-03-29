/*=======================================================================*/
/* eposcfg.cc                                                            */
/*                                                                       */
/* Desc: Tool to query EPOS configuration.                               */
/*                                                                       */
/* Parm: <configuration name>                                            */
/*                                                                       */
/* Auth: Davi Resner                                                     */
/*=======================================================================*/

// Traits are included in config.h
#include <system/config.h>

// Using only bare C to avoid conflicts with EPOS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace EPOS;
using namespace EPOS::S;
using namespace EPOS::S::U;

// Constants
const unsigned int TOKENS = 24;
const unsigned int COMPONENTS = 62;
const unsigned int STRING_SIZE = 128;

// Configuration tokens (integer tokens first, marked by INT_TOKENS)
char tokens[TOKENS][STRING_SIZE] = {
    "CPUS",
    "NODES",
    "MODE",
    "ARCH",
    "MACH",
    "MMOD",
    "CLOCK",
    "WORD_SIZE",
    "ENDIANESS",
    "MEM_BASE",
    "MEM_TOP",
    "MEM_SIZE",
    "MEM_SIZE_KB",
    "BOOT_STACK",
    "BOOT",
    "SETUP",
    "INIT",
    "APP_CODE",
    "APP_DATA",
    "SYS_CODE",
    "SYS_DATA",
    "BOOT_LENGTH_MIN",
    "BOOT_LENGTH_MAX",
    "EXPECTED_SIMULATION_TIME"
};

// Values for single-string tokens (populated at populate_strings())
char token_values[TOKENS][STRING_SIZE];

// List of EPOS components. Changes here must be replicated at populate_strings()
char components[COMPONENTS][STRING_SIZE] = {
    "CPU",
    "TSC",
    "MMU",
    "FPU",
    "PMU",
    "Machine",
    "PCI",
    "IC",
    "Timer",
    "RTC",
    "UART",
    "USB",
    "EEPROM",
    "Display",
    "Serial_Display",
    "Keyboard",
    "Serial_Keyboard",
    "Scratchpad",
    "GPIO",
    "I2C",
    "ADC",
    "FPGA",
    "NIC",
    "Ethernet",
    "IEEE802_15_4",
    "PCNet32",
    "RTL8139",
    "C905",
    "E100",
    "CC2538",
    "AT86RF",
    "GEM",
    "System",
    "Application",
    "Thread",
    "Active",
    "Periodic_Thread",
    "RT_Thread",
    "Task",
    "Scheduler",
    "Address_Space",
    "Segment",
    "Synchronizer",
    "Mutex",
    "Semaphore",
    "Condition",
    "Clock",
    "Chronometer",
    "Alarm",
    "Delay",
    "Network",
    "TSTP",
    "ARP",
    "IP",
    "ICMP",
    "UDP",
    "TCP",
    "DHCP",
    "IPC",
    "Link",
    "Port",
    "SmartData",
};

// List of enabled components (populated at populate_strings())
bool enabled_components[COMPONENTS];


// Prototypes
void populate_strings();
int set_token_value(const char * token, const char * value);
const char * get_token_value(const char * token);
int enable_component(const char * component);


// Main
int main(int argc, char **argv)
{
    populate_strings();

    if(argc > 2) {
        fprintf(stderr, "Usage: %s [-e|-h|<TOKEN>]\n", argv[0]);
        return -1;
    }

    if(argc == 1) {
        for(unsigned int i = 0; (i < TOKENS) && strlen(tokens[i]); i++)
            printf("%s = %s\n", tokens[i], token_values[i]);
        return 0;
    }

    if(!strcmp(argv[1], "-h")) {
        fprintf(stderr, "Usage: %s [-e|-h|<TOKEN>]\n", argv[0]);
        fprintf(stderr, "Possible values for <TOKEN> are:\n");
        for(unsigned int i = 0; (i < TOKENS) && strlen(tokens[i]); i++)
            fprintf(stderr, "%s\n", tokens[i]);
        return 0;
    }

    if(!strcmp(argv[1], "-e")) {
        for(unsigned int i = 0; i < COMPONENTS; i++)
            printf("%s = %i\n", components[i], enabled_components[i]);
        return 0;
    }

    if(argv[1][0] == '-') {
        fprintf(stderr, "Usage: %s [-e|-h|<TOKEN>]\n", argv[0]);
        return -1;
    }

    printf("%s", get_token_value(argv[1]));

    return 0;
}

// Populates the values for the string configurations
void populate_strings()
{

    // Integer value tokens
    char string[STRING_SIZE];

    snprintf(string, STRING_SIZE, "%i", Traits<Build>::CPUS);
    set_token_value("CPUS", string);

    snprintf(string, STRING_SIZE, "%i", Traits<Build>::NODES);
    set_token_value("NODES", string);

    snprintf(string, STRING_SIZE, "%i", Traits<CPU>::CLOCK);
    set_token_value("CLOCK", string);

    snprintf(string, STRING_SIZE, "%i", Traits<CPU>::WORD_SIZE);
    set_token_value("WORD_SIZE", string);

    snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::MEM_BASE);
    set_token_value("MEM_BASE", string);

    snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::MEM_TOP);
    set_token_value("MEM_TOP", string);

    snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::MEM_TOP - Traits<Machine>::MEM_BASE);
    set_token_value("MEM_SIZE", string);

    snprintf(string, STRING_SIZE, "0x%08x", (Traits<Machine>::MEM_TOP + 1 - Traits<Machine>::MEM_BASE) / 1024);
    set_token_value("MEM_SIZE_KB", string);

    if(Traits<Machine>::BOOT_STACK != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::BOOT_STACK);
    else
        string[0] = '\0';
    set_token_value("BOOT_STACK", string);

    if(Traits<Machine>::BOOT != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::BOOT);
    else
        string[0] = '\0';
    set_token_value("BOOT", string);

    if(Traits<Machine>::SETUP != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::SETUP);
    else
        string[0] = '\0';
    set_token_value("SETUP", string);

    if(Traits<Machine>::INIT != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::INIT);
    else
        string[0] = '\0';
    set_token_value("INIT", string);

    if(Traits<Machine>::APP_CODE != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::APP_CODE);
    else
        string[0] = '\0';
    set_token_value("APP_CODE", string);

    if(Traits<Machine>::APP_DATA != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::APP_DATA);
    else
        string[0] = '\0';
    set_token_value("APP_DATA", string);

    if(Traits<Machine>::SYS_CODE != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::SYS_CODE);
    else
        string[0] = '\0';
    set_token_value("SYS_CODE", string);

    if(Traits<Machine>::SYS_DATA != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "0x%08x", Traits<Machine>::SYS_DATA);
    else
        string[0] = '\0';
    set_token_value("SYS_DATA", string);

    if(Traits<Machine>::BOOT_LENGTH_MIN != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "%i", Traits<Machine>::BOOT_LENGTH_MIN);
    else
        string[0] = '\0';
    set_token_value("BOOT_LENGTH_MIN", string);

    if(Traits<Machine>::BOOT_LENGTH_MAX != Traits<Machine>::NOT_USED)
        snprintf(string, STRING_SIZE, "%i", Traits<Machine>::BOOT_LENGTH_MAX);
    else
        string[0] = '\0';
    set_token_value("BOOT_LENGTH_MAX", string);

    snprintf(string, STRING_SIZE, "%i", Traits<Build>::EXPECTED_SIMULATION_TIME);
    set_token_value("EXPECTED_SIMULATION_TIME", string);

    // String value tokens
    switch(Traits<Build>::MODE) {
    case Traits<Build>::LIBRARY:        set_token_value("MODE", "library");             break;
    case Traits<Build>::BUILTIN:        set_token_value("MODE", "builtin");             break;
    case Traits<Build>::KERNEL:         set_token_value("MODE", "kernel");              break;
    default:                            set_token_value("MODE", "unsuported");          break;
    }

    switch(Traits<Build>::ARCHITECTURE) {
    case Traits<Build>::AVR8:           set_token_value("ARCH", "avr8");        break;
    case Traits<Build>::H8:             set_token_value("ARCH", "h8");          break;
    case Traits<Build>::IA32:           set_token_value("ARCH", "ia32");        break;
    case Traits<Build>::X86_64:         set_token_value("ARCH", "x86_64");      break;
    case Traits<Build>::ARMv4:          set_token_value("ARCH", "armv4");       break;
    case Traits<Build>::ARMv7:          set_token_value("ARCH", "armv7");       break;
    case Traits<Build>::ARMv8:          set_token_value("ARCH", "armv8");       break;
    case Traits<Build>::SPARCv8:        set_token_value("ARCH", "sparc8");      break;
    case Traits<Build>::PPC32:          set_token_value("ARCH", "ppc32");       break;
    default:                            set_token_value("ARCH", "unsuported");  break;
    }

    switch(Traits<Build>::MACHINE) {
    case Traits<Build>::PC:             set_token_value("MACH", "pc");               break;
    case Traits<Build>::eMote1:         set_token_value("MACH", "emote1");           break;
    case Traits<Build>::eMote2:         set_token_value("MACH", "emote2");           break;
    case Traits<Build>::STK500:         set_token_value("MACH", "stk500");           break;
    case Traits<Build>::RCX:            set_token_value("MACH", "rcx");              break;
    case Traits<Build>::Leon:           set_token_value("MACH", "leon");             break;
    case Traits<Build>::Virtex:         set_token_value("MACH", "virtex");           break;
    case Traits<Build>::Cortex:         set_token_value("MACH", "cortex");         break;
    default:                            set_token_value("MACH", "unsuported");       break;
    }

    switch(Traits<Build>::MODEL) {
    case Traits<Build>::Unique:         set_token_value("MMOD", "unique");             break;
    case Traits<Build>::Legacy_PC:      set_token_value("MMOD", "legacy_pc");          break;
    case Traits<Build>::eMote3:         set_token_value("MMOD", "emote3");             break;
    case Traits<Build>::LM3S811:        set_token_value("MMOD", "lm3s811");            break;
    case Traits<Build>::Zynq:           set_token_value("MMOD", "zynq");               break;
    case Traits<Build>::Realview_PBX:   set_token_value("MMOD", "realview_pbx");       break;
    default:                            set_token_value("MMOD", "unsuported");         break;
    }

    switch(Traits<CPU>::ENDIANESS) {
    case Traits<CPU>::BIG:              set_token_value("ENDIANESS", "big");            break;
    case Traits<CPU>::LITTLE:           set_token_value("ENDIANESS", "little");         break;
    default:                            set_token_value("ENDIANESS", "undefined");      break;
    }


    // Enabled mediators
    if(Traits<CPU>::enabled)            enable_component("CPU");
    if(Traits<TSC>::enabled)            enable_component("TSC");
    if(Traits<MMU>::enabled)            enable_component("MMU");
    if(Traits<FPU>::enabled)            enable_component("FPU");
    if(Traits<PMU>::enabled)            enable_component("PMU");
    if(Traits<Machine>::enabled)        enable_component("Machine");
    if(Traits<PCI>::enabled)            enable_component("PCI");
    if(Traits<IC>::enabled)             enable_component("IC");
    if(Traits<Timer>::enabled)          enable_component("Timer");
    if(Traits<RTC>::enabled)            enable_component("RTC");
    if(Traits<UART>::enabled)           enable_component("UART");
    if(Traits<USB>::enabled)            enable_component("USB");
    if(Traits<EEPROM>::enabled)         enable_component("EEPROM");
    if(Traits<Display>::enabled)        enable_component("Display");
    if(Traits<Serial_Display>::enabled) enable_component("Serial_Display");
    if(Traits<Keyboard>::enabled)       enable_component("Keyboard");
    if(Traits<Serial_Keyboard>::enabled)enable_component("Serial_Keyboard");
    if(Traits<Scratchpad>::enabled)     enable_component("Scratchpad");
    if(Traits<GPIO>::enabled)           enable_component("GPIO");
    if(Traits<I2C>::enabled)            enable_component("I2C");
    if(Traits<ADC>::enabled)            enable_component("ADC");
    if(Traits<FPGA>::enabled)           enable_component("FPGA");
    if(Traits<Ethernet>::enabled)       enable_component("Ethernet");
    if(Traits<IEEE802_15_4>::enabled)   enable_component("IEEE802_15_4");
    if(Traits<PCNet32>::enabled)        enable_component("PCNet32");
    if(Traits<RTL8139>::enabled)        enable_component("RTL8139");
    if(Traits<C905>::enabled)           enable_component("C905");
    if(Traits<E100>::enabled)           enable_component("E100");
    if(Traits<CC2538>::enabled)         enable_component("CC2538");
    if(Traits<AT86RF>::enabled)         enable_component("AT86RF");
    if(Traits<GEM>::enabled)            enable_component("GEM");

    // Enabled components
    if(Traits<System>::enabled)         enable_component("System");
    if(Traits<Application>::enabled)    enable_component("Application");
    if(Traits<Thread>::enabled)         enable_component("Thread");
    if(Traits<Active>::enabled)         enable_component("Active");
    if(Traits<Periodic_Thread>::enabled)enable_component("Periodic_Thread");
    if(Traits<RT_Thread>::enabled)      enable_component("RT_Thread");
    if(Traits<Task>::enabled)           enable_component("Task");
    if(Traits<Scheduler<Thread>>::enabled)      enable_component("Scheduler");
    if(Traits<Address_Space>::enabled)  enable_component("Address_Space");
    if(Traits<Segment>::enabled)        enable_component("Segment");
    if(Traits<Synchronizer>::enabled)   enable_component("Synchronizer");
    if(Traits<Mutex>::enabled)          enable_component("Mutex");
    if(Traits<Semaphore>::enabled)      enable_component("Semaphore");
    if(Traits<Condition>::enabled)      enable_component("Condition");
    if(Traits<Clock>::enabled)          enable_component("Clock");
    if(Traits<Chronometer>::enabled)    enable_component("Chronometer");
    if(Traits<Alarm>::enabled)          enable_component("Alarm");
    if(Traits<Delay>::enabled)          enable_component("Delay");
    if(Traits<Network>::enabled)        enable_component("Network");
    if(Traits<TSTP>::enabled)           enable_component("TSTP");
    if(Traits<IP>::enabled)             enable_component("IP");
    if(Traits<ICMP>::enabled)           enable_component("ICMP");
    if(Traits<UDP>::enabled)            enable_component("UDP");
    if(Traits<TCP>::enabled)            enable_component("TCP");
    if(Traits<DHCP>::enabled)           enable_component("DHCP");
    if(Traits<IPC>::enabled)            enable_component("IPC");
    if(Traits<SmartData>::enabled)      enable_component("SmartData");
}

// Sets the value of a token if it exists
// returns the order in tokens if successful or -1 if token is not found
int set_token_value(const char * token, const char * value)
{
    unsigned int i;

    for(i = 0; i < TOKENS; i++)
        if(!strncmp(tokens[i], token, STRING_SIZE))
            break;

    if(i < TOKENS)
        strncpy(token_values[i], value, STRING_SIZE);
    else
        i = -1;

    return i;
}

// Gets the value of a token if it exists
// returns a pointer to the value if successful or 0 if not found
const char * get_token_value(const char * token)
{
    unsigned int i;

    for(i = 0; i < TOKENS; i++)
        if(!strncmp(tokens[i], token, STRING_SIZE))
            break;

    if(i < TOKENS)
        return token_values[i];
    else
        return 0;
}

// Sets a component as enabled
int enable_component(const char * component)
{
    unsigned int i;

    for(i = 0; i < COMPONENTS; i++)
        if(!strncmp(components[i], component, STRING_SIZE))
            break;

    if(i < COMPONENTS)
        enabled_components[i] = true;
    else
        i = -1;

    return i;
}

