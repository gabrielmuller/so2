// EPOS Realview PBX (Cortex-A9) Mediator Declarations

#ifndef __realview_pbx_h
#define __realview_pbx_h

#include <architecture/cpu.h>
#include <architecture/tsc.h>
#include <machine/rtc.h>
#include <system.h>

__BEGIN_SYS

class Realview_PBX
{
protected:
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;

public:
    static const unsigned int IRQS = 96;
    static const unsigned int TIMERS = 4;       // SP804 x 2
    static const unsigned int UARTS = 4;        // PL011
    static const unsigned int GPIO_PORTS = 3;   // PL061
    static const bool supports_gpio_power_up = false;

    // Base addresses for memory-mapped I/O devices
    enum {
        I2C_BASE                    = 0x10002000, // Versatile I2C
        AAC_BASE                    = 0x10004000, // PrimeCell PL041 Advanced Audio CODEC
        MMCI_BASE                   = 0x10005000, // PrimeCell PL181 MultiMedia Card Interface
        KBD0_BASE                   = 0x10006000, // PrimeCell PL050 PS2 Keyboard/Mouse Interface
        KBD1_BASE                   = 0x10007000, // PrimeCell PL050 PS2 Keyboard/Mouse Interface
        UART0_BASE                  = 0x10009000, // PrimeCell PL011 UART
        UART1_BASE                  = 0x1000a000, // PrimeCell PL011 UART
        UART2_BASE                  = 0x1000b000, // PrimeCell PL011 UART
        UART3_BASE                  = 0x1000c000, // PrimeCell PL011 UART
        GPIO0_BASE                  = 0x10013000, // PrimeCell PL061 GPIO
        GPIO1_BASE                  = 0x10014000, // PrimeCell PL061 GPIO
        GPIO2_BASE                  = 0x10015000, // PrimeCell PL061 GPIO
        RTC_BASE                    = 0x10017000, // PrimeCell PL031 RTC
        LCD_BASE                    = 0x10020000, // PrimeCell PL110 Color LCD Controller
        DMA_BASE                    = 0x10030000, // PrimeCell PL080 DMA Controller

        PPS_BASE                    = 0x1f000000, // A9 Private Peripheral Space
        SCU_BASE                    = 0x1f000000, // MP Snoop Control Unit
        GIC_CPU_BASE                = 0x1f000100,
        GLOBAL_TIMER_BASE           = 0x1f000200,
        PRIVATE_TIMER_BASE          = 0x1f000600,
        GIC_DIST_BASE               = 0x1f001000
    };

    // SCU Registers (as offsets to SCU_BASE)
    enum {                                      // Description              Type    Value after reset
        SCU_CTRL                    = 0x00,     // Control                  r/w     0x00000128
        SCU_CONFIG                  = 0x04,     // Configuration            r/w     0x00000000
        SCU_PSTAT                   = 0x08,     // Power Status             r/w     0x00000000
        SCU_IARSS                   = 0x0c      // Invalidate All Reg Secure State
    };

    // Useful bits in the CONTROL_REG0 register
    enum {                                      // Description              Type    Value after reset
        RXRES                       = 1 << 0,   // Reset Rx data path       r/w     0
        TXRES                       = 1 << 1,   // Reset Tx data path       r/w     0
        RXEN                        = 1 << 2,   // Receive enable           r/w     0
        TXEN                        = 1 << 4    // Transmit enable          r/w     0
    };

    // Useful bits in the MODE_REG0 register
    enum {                                      // Description              Type    Value after reset
        CHRL8                       = 0 << 1,   // Character Length 8 bits  r/w     0
        CHRL7                       = 2 << 1,   // Character Length 7 bits  r/w     0
        CHRL6                       = 3 << 1,   // Character Length 6 bits  r/w     0
        PAREVEN                     = 0 << 3,   // Even parity              r/w     0
        PARODD                      = 1 << 3,   // Odd parity               r/w     0
        PARNONE                     = 4 << 3,   // No parity                r/w     0
        NBSTOP2                     = 2 << 6,   // 2 stop bits              r/w     0
        NBSTOP1                     = 0 << 6,   // 1 stop bit               r/w     0
        CHMODENORM                  = 0 << 8,   // Normal mode              r/w     0
        CHMODELB                    = 2 << 8    // Loopback mode            r/w     0
    };

    // Useful bits in the INTRPT_EN_REG0, and INTRPT_DIS_REG0 registers
    enum {                                      // Description              Type    Value after reset
        INTRPT_RTRIG                = 1 << 0,   // Receiver FIFO empty      wo      0
        INTRPT_TTRIG                = 1 << 10   // Transmitter FIFO trigger wo      0
    };

    // Useful bits in the CHANNEL_STS_REG0 register
    enum {                                      // Description              Type    Value after reset
        STS_RTRIG                   = 1 << 0,   // Receiver FIFO trigger    ro      0
        STS_TFUL                    = 1 << 4    // Transmitter FIFO full    ro      0
    };

    // SLCR Registers offsets
    enum {                                      // Description
        SLCR_LOCK                   = 0x004,    // Lock the SLCR
        SLCR_UNLOCK                 = 0x008,    // Unlock the SLCR
        UART_CLK_CTRL               = 0x154,    // UART Ref Clock Control
        FPGA0_CLK_CTRL              = 0x170,    // PL Clock 0 Output control
        PSS_RST_CTRL                = 0x200,    // PS Software Reset Control
        FPGA_RST_CTRL               = 0x240     // FPGA Software Reset Control
    };

    // Useful bits in SLCR_LOCK
    enum {                                      // Description                  Type    Value after reset
        LOCK_KEY                    = 0x767B    // Lock key                     wo      0
    };

    // Useful bits in SLCR_UNLOCK
    enum {                                      // Description                  Type    Value after reset
        UNLOCK_KEY                  = 0xDF0D    // Unlock key                   wo      0
    };

    // Useful bits in FPGAN_CLK_CTRL
    enum {                                      // Description                  Type    Value after reset
        DIVISOR0                    = 1 << 8,   // First cascade divider        r/w     0x18
        DIVISOR1                    = 1 << 20   // Second cascade divider       r/w     0x1
    };

    // Global Timer Registers offsets
    enum {                                      // Description
        GTCTRL                      = 0x00,     // Low Counter
        GTCTRH                      = 0x04,     // High Counter
        GTCLR                       = 0x08,     // Control
        GTISR                       = 0x0C      // Interrupt Status
    };

    // Private Timer Registers offsets
    enum {                                      // Description
        PTLR                        = 0x00,     // Load
        PTCTR                       = 0x04,     // Counter
        PTCLR                       = 0x08,     // Control
        PTISR                       = 0x0C      // Interrupt Status
    };

    // Useful bits in PTCLR
    enum {                                      // Description                  Type    Value after reset
        TIMER_ENABLE                = 1 << 0,   // Enable                       r/w     0
        AUTO_RELOAD                 = 1 << 1,   // Auto reload                  r/w     0
        IRQ_EN                      = 1 << 2    // Enable interrupt             r/w     0
    };

    // Useful bits in PTISR
    enum {                                      // Description                  Type    Value after reset
        INT_CLR                     = 1 << 0    // Interrupt clear bit          r/w     0
    };

    // TTB definitions
    enum {
        TTBCR_DOMAIN                = 0x55555555, // All access to client
        TTB_DESCRIPTOR              = 0x10c0A
        // According to ARMv7 Arch. Ref. Manual: (description beggining at pages 1326 to 1329)
        // REG[19] NS, 0b0 for secure address space -> no effect on  Physical Address Space. -> Page 1330
        // REG[18] = 0b0 (mapped on 1 MB), it is not a supersection -> page 1329
        // REG[17] = 0b0 the TTB is global -> page 1378
        // REG[16] = 0b1 means shareable memory -> page 1368
        // REG[15] = 0b0 means read/write memory (if 1, read-only) -> ->  page 1358
        // REG[14:12] = TEX - 0b000 means possibly shareable page -> 1367
        // REG[11:10] = 0b11 means read/write with full access ->  page 1358
        // REG[9] = 0b0, Implementation Defined. -> page 1329
        // Reg[8:5] = 0b0000, Domain - not supported (DEPRECATED) its written on MCR p15, 0, <rt>, c3, c0, 0 -
        // REG[4] = XN = 0b0, means code can be executed. 0b1 stands for not exacutable area (generates Page Fault) -> page 1359
        // REG[3] = C (cacheable) = 1 with the config of TEX it means no Write-Allocate -> page 1367
        // REG[2] = B (Bufferable) = 0 with the config of TEX and C, it means Write-Through -> page 1367
        // Reg[1] = 0b1 means it points a section (more or equal than a MEGABYTE) -> Page 1329
        // REG[0] = PXN = 0b0 means every core can run, else processor running at PL1 generates Permission Fault-> page 1359
    };

protected:
    Realview_PBX() {}

    static void reboot() {}

    static unsigned int cpu_id() {
        int id;
        ASM("mrc p15, 0, %0, c0, c0, 5" : "=r"(id) : : );
        return id & 0x3;
    }

    static unsigned int n_cpus() {
        int n;
        ASM("mrc p15, 4, %0, c15, c0, 0 \t\n\
             ldr %0, [%0, #0x004]" : "=r"(n) : : );
        return (n & 0x3) + 1;
    }

    static void smp_init(unsigned int n_cpus);

    static void enable_uart(unsigned int unit) {}

    // Power Management
    static void power_uart(unsigned int unit, const Power_Mode & mode) {
        assert(unit < UARTS);
        switch(mode) {
        case ENROLL:
            break;
        case DISMISS:
            break;
        case SAME:
            break;
        case FULL:
            break;
        case LIGHT:
            break;
        case SLEEP:
            break;
        case OFF:
            break;
        }
    }

    static void power_user_timer(unsigned int unit, const Power_Mode & mode) {
        assert(unit < UARTS);
        switch(mode) {
        case ENROLL:
            break;
        case DISMISS:
            break;
        case SAME:
            break;
        case FULL:
            break;
        case LIGHT:
        case SLEEP:
        case OFF:
            break;
        }
    }

    static void branch_prediction_enable() {
        ASM("                                                                           \t\n\
            MRC     p15, 0, r0, c1, c0, 0                  // Read SCTLR                \t\n\
            ORR     r0, r0, #(1 << 11)                     // Set the Z bit (bit 11)    \t\n\
            MCR     p15, 0,r0, c1, c0, 0                   // Write SCTLR               \t\n\
            ");
    }

    static void enable_dside_prefetch() {
        ASM("MRC p15, 0, r0, c1, c0, 1 \t\n\
             ORR r0, r0, #(0x1 << 2)   \t\n\
             MCR p15, 0, r0, c1, c0, 1 ");
    }

    static void invalidate_tlb() {
        ASM("MOV r0, #0x0 \t\n MCR p15, 0, r0, c8, c7, 0"); // TLBIALL - Invalidate entire Unifed TLB
    }

    static void clear_branch_prediction_array() {
        ASM("MOV r0, #0x0 \t\n MCR p15, 0, r0, c7, c5, 6"); // BPIALL - Invalidate entire branch predictor array
    }

    static void set_domain_access() {
        ASM("MCR p15, 0, %0, c3, c0, 0" : : "p"(Realview_PBX::TTBCR_DOMAIN) :);
    }

    static void enable_scu() {
        scu(0) |= 0x1;
    }

    static void secure_scu_invalidate() {
        // void secure_SCU_invalidate(unsigned int cpu, unsigned int ways)
        // cpu: 0x0=CPU 0 0x1=CPU 1 etc...
        // This function invalidates the SCU copy of the tag rams
        // for the specified core.  Typically only done at start-up.
        // Possible flow:
        //- Invalidate L1 caches
        //- Invalidate SCU copy of TAG RAMs
        //- Join SMP
        Reg32 _cpu = cpu_id();
        _cpu = _cpu << 2; // Convert into bit offset (four bits per core)
        Reg32 _ways = 0x0F; // all four ways
        _ways = _ways << _cpu; // Shift ways into the correct CPU field
        scu(SCU_IARSS) = _ways;
    }

    static void scu_enable_cache_coherence() {
        scu(SCU_CONFIG) |= 0xF0;
    }

    static void enable_maintenance_broadcast() {
        // Enable the broadcasting of cache & TLB maintenance operations
        // When enabled AND in SMP, broadcast all "inner sharable"
        // cache and TLM maintenance operations to other SMP cores
        ASM("MRC     p15, 0, r0, c1, c0, 1   // Read ACTLR.             \t\n\
             ORR     r0, r0, #(0x01)         // Set the FW bit (bit 0). \t\n\
             MCR     p15, 0, r0, c1, c0, 1   // Write ACTLR."
        );
    }

    static void join_smp() {
        ASM("                                                                       \t\n\
            MRC     p15, 0, r0, c1, c0, 1   // Read ACTLR                           \t\n\
            ORR     r0, r0, #(0x01 << 6)    // Set SMP bit                          \t\n\
            MCR     p15, 0, r0, c1, c0, 1   // Write ACTLR                          \t\n\
            ");
    }

    static void page_tables_setup() {
        Reg32 aux = 0x0;
        for (int curr_page = 4095; curr_page > 0; curr_page--) {
            aux = TTB_DESCRIPTOR | (curr_page << 20);
            reinterpret_cast<volatile Reg32 *>(Traits<Machine>::PAGE_TABLES)[curr_page] = aux;
        }
        reinterpret_cast<volatile Reg32 *>(Traits<Machine>::PAGE_TABLES)[0] = TTB_DESCRIPTOR;
    }

    static void enable_mmu() {
        // TTB0 size is 16 kb, there is no TTB1 and no TTBCR
        // ARMv7 Architecture Reference Manual, pages 1330
        ASM ("MOV r0, #0x0 \t\n MCR p15, 0, r0, c2, c0, 2"); // Write Translation Table Base Control Register.
        ASM ("MCR p15, 0, %0, c2, c0, 0" : : "p"(Traits<Machine>::PAGE_TABLES) :); // Write Translation Table Base Register 0.

        // Enable MMU
        //-------------
        //0     - M, set to enable MMU
        // Leaving the caches disabled until after scatter loading.
        ASM ("                                                                      \t\n\
            MRC     p15, 0, r0, c1, c0, 0       // Read current control reg         \t\n\
            ORR     r0, r0, #(0x1 << 2)         // The C bit (data cache).          \t\n\
            ORR     r0, r0, #(0x1 << 12)        // The I bit (instruction cache).   \t\n\
            ORR     r0, r0, #0x01               // Set M bit                        \t\n\
            MCR     p15, 0, r0, c1, c0, 0       // Write reg back                   \t\n\
            ");
    }

    static void clear_bss() {
        Reg32 bss_start, bss_end;
        ASM("LDR %0, =__bss_start__" : "=r"(bss_start) :);
        ASM("LDR %0, =__bss_end__" : "=r"(bss_end) :);
        Reg32 limit = (bss_end - bss_start)/4;
        for(Reg32 i = 0; i < limit; i++) {
            reinterpret_cast<volatile Reg32 *>(bss_start)[i] = 0x0;
        }
    }

public:
    static volatile Reg32 & scu(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(SCU_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & global_timer(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GLOBAL_TIMER_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & private_timer(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(PRIVATE_TIMER_BASE)[o / sizeof(Log_Addr)]; }

    static void pre_init();
    static void init();
};

typedef Realview_PBX Machine_Model;

__END_SYS

#endif
