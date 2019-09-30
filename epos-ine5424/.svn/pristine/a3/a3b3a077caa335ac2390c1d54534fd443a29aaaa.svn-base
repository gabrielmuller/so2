// EPOS Realview PBX (Cortex-A9) Initialization

#include <system/config.h>
#include <machine.h>
#include <architecture/cpu.h>
#include __MODEL_H
#ifdef __realview_pbx_h

__BEGIN_SYS

// DSB causes completion of all cache maintenance operations appearing in program
// order before the DSB instruction.
void dsb()
{
    ASM("dsb");
}

// An ISB instruction causes the effect of all branch predictor maintenance
// operations before the ISB instruction to be visible to all instructions
// after the ISB instruction.
void isb()
{
    ASM("isb");
}

void invalidate_caches()
{
    ASM("                  \t\n\
.invalidate_caches:                                                                                   \t\n\
        push    {r4-r12}                                                                              \t\n\
        // Based on code example given in section b2.2.4/11.2.4 of arm ddi 0406b                      \t\n\
        mov     r0, #0                                                                                \t\n\
        // ICIALLU - invalidate entire i cache, and flushes branch target cache                       \t\n\
        mcr     p15, 0, r0, c7, c5, 0                                                                 \t\n\
        mrc     p15, 1, r0, c0, c0, 1      // read CLIDR                                              \t\n\
        ands    r3, r0, #0x7000000                                                                    \t\n\
        mov     r3, r3, lsr #23            // cache level value (naturally aligned)                   \t\n\
        beq     .invalidate_caches_finished                                                           \t\n\
        mov     r10, #0                                                                               \t\n\
                                                                                                      \t\n\
.invalidate_caches_loop1:                                                                             \t\n\
        add     r2, r10, r10, lsr #1       // work out 3xcachelevel                                   \t\n\
        mov     r1, r0, lsr r2             // bottom 3 bits are the cache type for this level         \t\n\
        and     r1, r1, #7                 // get those 3 bits alone                                  \t\n\
        cmp     r1, #2                                                                                \t\n\
        blt     .invalidate_caches_skip    // no cache or instruction at this level                   \t\n\
        mcr     p15, 2, r10, c0, c0, 0     // write the cache size selection register                 \t\n\
        isb                                // ISB to sync the change to the cachesizeid reg           \t\n\
        mrc     p15, 1, r1, c0, c0, 0      // reads current cache size id register                    \t\n\
        and     r2, r1, #0x7               // extract the line length field                           \t\n\
        add     r2, r2, #4                 // add 4 for the line length offset (log2 16 bytes)        \t\n\
        ldr     r4, =0x3ff                                                                            \t\n\
        ands    r4, r4, r1, lsr #3         // r4 is the max number on way size (right algn)           \t\n\
        clz     r5, r4                     // r5 is the bit position of the way size increment        \t\n\
        ldr     r7, =0x00007fff                                                                       \t\n\
        ands    r7, r7, r1, lsr #13        // r7 is max number of index size (right algn)             \t\n\
                                                                                                      \t\n\
.invalidate_caches_loop2:                                                                             \t\n\
        mov     r9, r4                     // r9 working copy of the max way size (right algn)        \t\n\
                                                                                                      \t\n\
.invalidate_caches_loop3:                                                                             \t\n\
        orr     r11, r10, r9, lsl r5       //factor in way and cache number into r11                  \t\n\
        orr     r11, r11, r7, lsl r2       // factor in the index number                              \t\n\
        mcr     p15, 0, r11, c7, c6, 2     // DCISW - invalidate by set/way                           \t\n\
        subs    r9, r9, #1                 // decrement the way number                                \t\n\
        bge     .invalidate_caches_loop3                                                              \t\n\
        subs    r7, r7, #1                 // decrement the index                                     \t\n\
        bge     .invalidate_caches_loop2                                                              \t\n\
                                                                                                      \t\n\
.invalidate_caches_skip:                                                                              \t\n\
        add     r10, r10, #2               // increment the cache number                              \t\n\
        cmp     r3, r10                                                                               \t\n\
        bgt     .invalidate_caches_loop1                                                              \t\n\
                                                                                                      \t\n\
.invalidate_caches_finished:                                                                          \t\n\
        pop     {r4-r12}                                                                              \t\n\
        ");
}

void Realview_PBX::pre_init()
{
    // Relocated the vector table
    ASM("MCR p15, 0, %0, c12, c0, 0" : : "p"(Traits<Machine>::VECTOR_TABLE) :);

    // MMU init
    invalidate_caches();
    clear_branch_prediction_array();
    invalidate_tlb();
    enable_dside_prefetch();
    set_domain_access();
    dsb();
    isb();

    // Initialize PageTable.

    // Create a basic L1 page table in RAM, with 1MB sections containing a flat
    // (VA=PA) mapping, all pages Full Access, Strongly Ordered.

    // It would be faster to create this in a read-only section in an assembly file.

    if(Machine::cpu_id() == 0)
        page_tables_setup();

    // Activate the MMU
    enable_mmu();
    dsb();
    isb();

    // MMU now enable - Virtual address system now active

    // Branch Prediction init
    branch_prediction_enable();

    // SMP initialization
    if(Machine::cpu_id() == 0) {
        //primary core
        enable_scu();
        secure_scu_invalidate();
        join_smp();
        enable_maintenance_broadcast();
        scu_enable_cache_coherence();

        // gic enable is now on Machine::pre_init()

        // FIXME:: is the following part of memory map or is there really a flat segment register?
        // this is only a flat segment register that allows mapping the start point for the secondary cores
        static const unsigned int FLAG_SET_REG                = 0x10000030;
        // set flag register to the address secondary CPUS are meant to start executing
        ASM("str %0, [%1]" : : "p"(Traits<Machine>::VECTOR_TABLE), "p"(FLAG_SET_REG) :);

        // secondary cores reset is now on Machine::pre_init()

        clear_bss();
    } else {
        //clear_interrupt();
        secure_scu_invalidate();
        join_smp();
        enable_maintenance_broadcast();
        scu_enable_cache_coherence();
    }
}

void Realview_PBX::init()
{
}

void Realview_PBX::smp_init(unsigned int n_cpus) {
    CPU::Reg32 interrupt_id = 0x0; // reset id
    
    CPU::Reg32 target_list = 0x0;
    if(n_cpus <= Machine::n_cpus()) {
        for(unsigned int i = 0; i < n_cpus; i++) {
            target_list |= 1 << (i);
        }
    } else {
        for(unsigned int i = 0; i < Machine::n_cpus(); i++) {
            target_list |= 1 << (i);
        }
    }
    CPU::Reg32 filter_list = 0x01; // except the current
    
    IC::send_sgi(interrupt_id, target_list, filter_list);
}

__END_SYS
#endif
