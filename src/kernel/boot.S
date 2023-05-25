#include "arm64/sysregs.h"
#include "kernel/mm.h"
#include "kernel/early_mini_uart.inc"

#define RODATA_STR(label, msg)                  \
.pushsection .rodata.str, "aMS", %progbits, 1 ; \
label:  .asciz msg;                             \
.popsection

/* Lower attributes for Translation Table Entry
 * nG: not global. TLB entry apply to all ASID.
 * AF: Access flag. Indicate whether an entry is accessed for first time since it was set to 0 (by HW?).
 * SH: Sharability. 11 = Inner Shareable, 10 = Outer Shareable
 * NS: non secure.
 * ATTR: Memory attribute index used as a 8-bit bitfield into 64-bit MAIR_ELx
 * T: table flag. Indicate whether an entry is a table entry (point to next-level table) or a block entry (point to PA)
 * P: valid flag.
 */
#define PT_PT           0xf6b /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=010 T=1 P=1 */ 
#define PT_NORMAL       0xf69 /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=010 T=0 P=1 */ 
#define PT_NORMAL_L3    0xf6b /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=010 T=1 P=1 */ 
#define PT_DEV          0xe65 /* nG=1 AF=1 SH=10 AP=01 NS=1 ATTR=001 T=0 P=1 */
#define PT_DEV_L3       0xe67 /* nG=1 AF=1 SH=10 AP=01 NS=1 ATTR=001 T=1 P=1 */

/* Convenience defines to get slot used by VA mapping. */
#define HYP_LEVEL_0_INDEX   level_0_table_offset(HYP_VIRT_START)
#define HYP_LEVEL_1_INDEX   level_1_table_offset(HYP_VIRT_START)
#define HYP_LEVEL_2_INDEX   level_2_table_offset(HYP_VIRT_START)
#define HYP_LEVEL_3_INDEX   level_3_table_offset(HYP_VIRT_START)

/*
 * Macro to print a string to the UART, if there is one.
 *
 * Clobbers x0 - x3
 */
#define PRINT(_s)          \
        mov   x3, lr ;     \
        adr   x0, 98f ;    \
        bl    puts    ;    \
        mov   lr, x3 ;     \
        RODATA_STR(98, _s)

/*
 * Macro to print the value of register \xb
 *
 * Clobbers x0 - x4
 */
.macro print_reg xb
        mov   x0, \xb
        mov   x4, lr
        bl    putn
        mov   lr, x4
.endm

/*
 * Pseudo-op for PC relative adr <reg>, <symbol> where <symbol> is
 * within the range +/- 4GB of the PC.
 *
 * @dst: destination register (64 bit wide)
 * @sym: name of the symbol
 * See: https://stackoverflow.com/questions/41906688/what-are-the-semantics-of-adrp-and-adrl-instructions-in-arm-assembly
 */
.macro  adr_l, dst, sym
        adrp \dst, \sym
        add  \dst, \dst, :lo12:\sym
.endm

/* Load the physical address of a symbol into xb */
.macro load_paddr xb, sym
        ldr \xb, =\sym
        add \xb, \xb, x20
.endm

.section ".text.boot"  // Make sure the linker puts this at the start of the kernel image

.global _start  // Execution starts here

// Entry point for the kernel. Registers:
// x0 -> 32 bit pointer to DTB in memory (primary core only) / 0 (secondary cores)
// x1 -> 0
// x2 -> 0
// x3 -> 0
// x4 -> 32 bit kernel entry point, _start location
_start:
    msr     DAIFSet, 0xf           /* Disable all interrupts */

    /* Save the bootloader arguments in less-clobberable registers */
    mov     x21, x0                /* x21 := DTB, physical address  */

    /* Find out where we are */
    ldr     x0, =_start
    adr     x19, _start            /* adds value of `_start` addr to PC and store to x19. x19 := paddr (start) */
    sub     x20, x19, x0           /* x20 := phys-offset */

    bl      init_uart
    PRINT("- Boot CPU booting -\r\n")

    bl      check_cpu_mode
    bl      cpu_init
    bl      create_boot_page_tables
    ldr     x4, =primary_switched
    bl      enable_mmu
    br      x4
primary_switched:
    /*
     * The 1:1 map may clash with other parts of virtual memory
     * layout. As it is not used anymore, remove it completely to
     * avoid having to worry about replacing existing mapping
     * afterwards.
     */
    bl      remove_identity_map
    bl      setup_fixmap
    ldr     x23, =EARLY_UART1_VIRTUAL_ADDRESS

    // Clean the BSS section
    PRINT("- Zero BSS -\r\n")
    ldr     x0, =__bss_start     // Start address
    ldr     x1, =__bss_end       // Size of bss
1:  str     xzr, [x0], #8
    cmp     x0, x1
    b.lo    1b               // Loop if non-zero
    
    // PRINT("- Booting is done!! -\r\n")
    // Set up the arguments again and jump to our main() routine in C (make sure it doesn't return) 
    mov     x0, x20
    mov     x1, x21

    // Set up our stack
    ldr     x2, =boot_stack
    add     x2, x2, #STACK_SIZE
    mov     sp, x2
    ldr     x3, =kernel_main
    br      x3

check_cpu_mode:
    PRINT("- Current EL ")
    mrs     x5, CurrentEL
    print_reg x5
    PRINT(" -\r\n")
    /* Are we in EL2 */
    cmp     x5, #PSR_MODE_EL2t
    ccmp    x5, #PSR_MODE_EL2h, #0x4, ne
    b.ne    1f /* No */
    ret
1:
    /* OK, we're boned. */
    PRINT("- We must be entered in NS EL2 mode -\r\n")
    b       fail
    ret   

/*
 * Initialize the processor for turning the MMU on.
 *
 * Clobbers x0 - x3
 */
cpu_init:
    /* Set up memory attribute type tables */
    ldr   x0, =MAIRVAL
    msr   mair_el2, x0

    PRINT("- Initialize CPU -\r\n")
    /*
     * Set up TCR_EL2:
     * PS -- Based on ID_AA64MMFR0_EL1.PARange
     * Top byte is used
     * PT walks use Inner-Shareable accesses,
     * PT walks are write-back, write-allocate in both cache levels,
     * 48-bit virtual address space goes through this table.
     */
    
    ldr   x0, =(TCR_RES1|TCR_SH0_IS|TCR_ORGN0_WBWA|TCR_IRGN0_WBWA|TCR_T0SZ(64-48))
    /* ID_AA64MMFR0_EL1[3:0] (PARange) corresponds to TCR_EL2[18:16] (PS) */
    mrs   x1, ID_AA64MMFR0_EL1
    /* Limit to 48 bits, 256TB PA range (#5) */
    ubfm  x1, x1, #0, #3    // get ID_AA64MMFR0_EL1[3:0]
    mov   x2, #5
    cmp   x1, x2            // compare to 256TB (#5)
    csel  x1, x1, x2, lt    // and selects the min value

    /* Put PA Range into TCR_EL2[18:16] */
    bfi   x0, x1, #16, #3   // 3 bits in x0, starting at bit 16, are replaced by 3 bits from x1, starting at bit[0]. Other bits in x0 are unchanged.

    msr   tcr_el2, x0

    // Default SCTLR: Little-endian, no dcache, icache enabled, no mmu
    ldr    x0, =SCTLR_VALUE_MMU_DISABLED
    msr    SCTLR_EL2, x0
    isb

    /*
     * Ensure that any exceptions encountered at EL2
     * are handled using the EL2 stack pointer, rather
     * than SP_EL0.
     */
    msr spsel, #1
    ret

/*
 * Macro to find the index at a given page-table level
 *
 * index:           index computed
 * virt:            virtual address we want to map
 * lvl:             page-table level
 */
.macro get_table_index, index, virt, lvl
    ubfx    \index, \virt, #PT_LEVEL_SHIFT(\lvl), #PT_LEVEL_BITS
.endm

/*
 * Macro to create a page table entry in \from_tbl to \to_tbl
 *
 * from_tbl:        table symbol where the entry will be created
 * to_tbl:          table symbol to point to
 * virt:   virtual address we want to map
 * lvl:             page-table level
 * tmp1:            scratch register
 * tmp2:            scratch register
 * tmp3:            scratch register
 *
 * Preserves \virt
 * Clobbers \tmp1, \tmp2, \tmp3
 *
 */
.macro create_table_entry, from_tbl, to_tbl, virt, lvl, tmp1, tmp2, tmp3
    get_table_index \tmp1, \virt, \lvl /* Get index from VA virt at lvl's table */

    /* Get address of to_tbl and fill in the memory attributes */
    load_paddr \tmp2, \to_tbl
    mov     \tmp3, #PT_NORMAL_L3
    orr     \tmp3, \tmp3, \tmp2
    
    /* Load address to from_tbl */
    adr_l   \tmp2, \from_tbl

    /* Store the table entry to to_tbl in from_tbl */
    str     \tmp3, [\tmp2, \tmp1, lsl #3]

.endm

/*
 * Macro to create a mapping entry in \tbl to \phys. Only mapping in 3rd
 * level table (i.e page granularity) is supported.
 *
 * ptbl:    table symbol where the entry will be created
 * virt:    virtual address
 * phys:    physical address (should be page aligned)
 * tmp1:    scratch register
 * tmp2:    scratch register
 * tmp3:    scratch register
 * type:    mapping type. If not specified it will be normal memory (PT_NORMAL_L3)
 *
 * Preserves \virt, \phys
 * Clobbers \tmp1, \tmp2, \tmp3
 *
 * Note that all parameters using registers should be distinct.
 */
.macro create_mapping_entry, ptbl, virt, phys, tmp1, tmp2, tmp3, type=PT_NORMAL_L3
        and   \tmp3, \phys, #PT_LEVEL_3_MASK     /* \tmp3 := PAGE_ALIGNED(phys) */

        get_table_index \tmp1, \virt, 3      /* \tmp1 := slot in \tlb */

        mov   \tmp2, #\type                 /* \tmp2 := right for section PT */
        orr   \tmp2, \tmp2, \tmp3           /*          + PAGE_ALIGNED(phys) */

        adr_l \tmp3, \ptbl

        str   \tmp2, [\tmp3, \tmp1, lsl #3]
.endm

/*
 * Build the boot pagetable's first-level entries. The structure
 * is described in mm.h.
 *
 * Inputs:
 *   x19: paddr(start)
 *   x20: phys offset
 *
 * Clobbers x0 - x5
 */
create_boot_page_tables:
    /* Prepare the page tables */
    ldr     x0, =HYP_VIRT_START
    create_table_entry boot_level_0, boot_level_1, x0, 0, x1, x2, x3
    create_table_entry boot_level_1, boot_level_2, x0, 1, x1, x2, x3
    create_table_entry boot_level_2, boot_level_3, x0, 2, x1, x2, x3

    /* Map ourselves */
    // adr_l   x4, boot_level_3

////////////////////////// Need to map the text and do it 1:1
/*
    mov     x5, xzr
    mov     x0, x19
1:  create_mapping_entry boot_level_3, x0, x0, x1, x2, x3
    add     x0, x0, #PAGE_SIZE
    add     x5, x5, #8
    cmp     x5, #(PT_ENTRIES << 3)
    b.lt    1b
*/

    // Xen source for reference, map text at beginning of L3 (0th slot). 
    // But this ONLY WORKS if HYP_VIRT_START is aligned to 0x0020000 (2MB boundary which aligns with L3 size)

    /*
    lsr     x2, x19, #PT_LEVEL_3_SHIFT
    lsl     x2, x2, #PT_LEVEL_3_SHIFT
    mov     x3, #PT_NORMAL_L3
    orr     x2, x2, x3                  // Populate attributes for the mapping entry of paddr(_start)  
    
    // map vaddr(_start) in level 3 tbl
    mov     x1, xzr                     // Offset starts at 0
1:  str     x2, [x4, x1]                // Loads paddr(_start) into pgtbl at offset
    add     x2, x2, #PAGE_SIZE          // Next page
    add     x1, x1, #8                  // Next slot is 8 bytes (64-bit) ahead, since we wrote an address
    cmp     x1, #(PT_ENTRIES << 3)      // 512 entries per page
    b.lt    1b
    */

    get_table_index x1, x0, 3                               // Get L3 index of HYP_VIRT_START into x1
    mov     x5, x19                                         // Copy paddr(_start) to x5
1:  create_mapping_entry boot_level_3, x0, x5, x2, x3, x4   // Map virt in x0 to phys in x5
    add     x0, x0, #PAGE_SIZE                              // Move VA by 1 page
    add     x5, x5, #PAGE_SIZE                              // Move PA by 1 page
    add     x1, x1, #1                                      // Add 1 to index
    cmp     x1, #PT_ENTRIES                                 // Stop when we reached L3 boundary, we should have mapped 1.5KB of memory
    b.lt    1b


    // If we're not loaded at exactly HYP_VIRT_START, we would need an additional 1:1 mapping
    cmp     x19, x0
    bne     handle_identity_mapping
    ret

handle_identity_mapping:
    /*
     * Setup the 1:1 mapping so we can turn the MMU on. Note that
     * only the first page will be part of the 1:1 mapping.
     */

    PRINT("- Creating 1:1 map -\r\n")

    /* Find where paddr(_start) is mapped to in L0 table
     * If it clashes with our VA map of HYP_VIRT_START in L0,
     * then try to map in L1 table
     */
    get_table_index x0, x19, 0
    cmp     x0, #HYP_LEVEL_0_INDEX
    beq     1f
    // If index for paddr(_start) doesn't clash, we can start our 1:1 map from L0
    PRINT("- Mapping 1:1 from L0 -\r\n")
    create_table_entry boot_level_0, boot_id_level_1, x19, 0, x0, x1, x2
    b       link_id_map_from_l1

1:
    /* Find where paddr(_start) is mapped to in L1 table
     * If it clashes with our VA map of HYP_VIRT_START in L1,
     * then try to map in L2 table
     */
    get_table_index x0, x19, 1
    cmp     x0, #HYP_LEVEL_1_INDEX
    beq     1f
    // If index for paddr(_start) doesn't clash, we can start our 1:1 map from L1
    PRINT("- Mapping 1:1 from L1 -\r\n")
    create_table_entry boot_level_1, boot_id_level_2, x19, 1, x0, x1, x2
    b       link_id_map_from_l2

1:
    /* Find where paddr(_start) is mapped to in L2 table
     * If it clashes with our VA map of HYP_VIRT_START in L2,
     * then try to map in L3 table
     */
    get_table_index x0, x19, 2
    cmp     x0, #HYP_LEVEL_2_INDEX
    beq     1f
    // If index for paddr(_start) doesn't clash, we can start our 1:1 map from L2
    PRINT("- Mapping 1:1 from L2 -\r\n")
    create_table_entry boot_level_2, boot_id_level_3, x19, 2, x0, x1, x2
    b       link_id_map_from_l3

1:
    /* Find where paddr(_start) is mapped to in L3 table
     * If it clashes with our VA map of HYP_VIRT_START in L3,
     * we're screwed since we have nowhere else to map.
     */
    get_table_index x0, x19, 3
    cmp     x0, #HYP_LEVEL_3_INDEX
    beq     virt_phys_clash
    // If index for paddr(_start) doesn't clash, we can start our 1:1 map from L3
    PRINT("- Mapping 1:1 from L3 -\r\n")
    create_mapping_entry boot_level_3, x19, x19, x0, x1, x2
    ret

link_id_map_from_l1:
    create_table_entry boot_id_level_1, boot_id_level_2, x19, 1, x0, x1, x2
link_id_map_from_l2:
    create_table_entry boot_id_level_2, boot_id_level_3, x19, 2, x0, x1, x2
link_id_map_from_l3:
    create_mapping_entry boot_id_level_3, x19, x19, x0, x1, x2
    ret

virt_phys_clash:
    PRINT("- Unable to build boot page tables - virt and phys addresses clash -\r\n")
    PRINT("- Try mapping HYP_VIRT_START at a different page-aligned address -\r\n")
    b       fail

/*
 * Turn on the Data Cache and the MMU. The function will return on the 1:1
 * mapping. In other word, the caller is responsible to switch to the runtime
 * mapping.
 *
 * Clobbers x0 - x3
 */
enable_mmu:
    PRINT("- Turning on paging -\r\n")
    /*
     * The state of the TLBs is unknown before turning on the MMU.
     * Flush them to avoid stale one.
     */
    tlbi    alle2                  /* Flush hypervisor TLBs */
    dsb     nsh

    // Write our PT's paddr to TTBR0_EL2
    load_paddr x0, boot_level_0
    msr     TTBR0_EL2, x0

    // Turn on MMU and D-Cache
    mrs     x0, SCTLR_EL2
    orr     x0, x0, #SCTLR_MMU_ENABLED
    orr     x0, x0, #SCTLR_D_CACHE_ENABLED
    dsb     sy              // Flush PTE writes and finish reads
    msr     SCTLR_EL2, x0   // Enable paging
    isb                     // Flush icache
    ret

/*
 * Remove the 1:1 map from the page-tables. It is not easy to keep track
 * where the 1:1 map was mapped, so we will look for the top-level entry
 * exclusive to the 1:1 map and remove it.
 *
 * Inputs:
 *   x19: paddr(_start)
 *
 * Clobbers x0 - x1
 */
remove_identity_map:
    /*
     * Find index of paddr(_start) in L0 table. If it clashes with HYP_VIRT_START index,
     * the 1:1 map didn't start from L0, continue to check L1. If it does not clash,
     * remove the 1:1 map from L0
     */
    get_table_index x1, x19, 0
    cmp     x1, #HYP_LEVEL_0_INDEX
    beq     1f
    /* It didn't clash at L0, so it was mapped here. Remove it */
    ldr     x0, =boot_level_0
    str     xzr, [x0, x1, lsl #3]
    b       identity_map_removed

1:
    /*
     * Next check L1.
     * Find index of paddr(_start) in L1 table. If it clashes with HYP_VIRT_START index,
     * the 1:1 map didn't start from L1, continue to check L2. If it does not clash,
     * remove the 1:1 map from L1
     */
    get_table_index x1, x19, 1
    cmp     x1, #HYP_LEVEL_1_INDEX
    beq     1f
    /* It didn't clash at L1, so it was mapped here. Remove it */
    ldr     x0, =boot_level_1
    str     xzr, [x0, x1, lsl #3]
    b       identity_map_removed

1:
    /*
     * Next check L2.
     * Find index of paddr(_start) in L2 table. If it clashes with HYP_VIRT_START index,
     * the 1:1 map didn't start from L2, continue to check L3. If it does not clash,
     * remove the 1:1 map from L2
     */
    get_table_index x1, x19, 2
    cmp     x1, #HYP_LEVEL_2_INDEX
    beq     1f
    /* It didn't clash at L2, so it was mapped here. Remove it */
    ldr     x0, =boot_level_2
    str     xzr, [x0, x1, lsl #3]
    b       identity_map_removed

1:
    /*
     * Next check L3.
     * Find index of paddr(_start) in L3 table. If it clashes with HYP_VIRT_START index,
     * the 1:1 map was not mapped at all. If it does not clash, remove the 1:1 map from L3
     */
    get_table_index x1, x19, 3
    cmp     x1, #HYP_LEVEL_3_INDEX
    beq     1f
    /* It didn't clash at L3, so it was mapped here. Remove it */
    ldr     x0, =boot_level_3
    str     xzr, [x0, x1, lsl #3]
    b       identity_map_removed

identity_map_removed:
    dsb   nshst
    tlbi  alle2
    dsb   nsh
    isb

    ret

/*
 * Map the UART in the fixmap and hook the fixmap table in the page tables.
 *
 * The fixmap cannot be mapped in create_boot_page_tables because it may
 * clash with the 1:1 mapping.
 *
 * Inputs:
 *   x20: Physical offset
 *   x23: Early UART base physical address
 *
 * Clobbers x0 - x3
 */
setup_fixmap:
        /* Add UART to the fixmap table */
        ldr   x0, =EARLY_UART1_VIRTUAL_ADDRESS
        create_mapping_entry hyp_fixmap, x0, x23, x1, x2, x3, type=PT_NORMAL_L3

        /* Map fixmap into L2 table */
        ldr   x0, =FIXMAP_ADDR(FIXMAP_UART)
        create_table_entry boot_level_2, hyp_fixmap, x0, 2, x1, x2, x3
        /* Ensure any page table updates made above have occurred. */
        dsb   nshst

        ret
 
/*
 * Initialize the UART. Should only be called on the boot CPU.
 *
 * Output:
 *  x23: Early UART base physical address
 *
 * Clobbers x0 - x1
 */
init_uart:
    ldr     x23, =AUX_BASE
    ldr     x24, =GPIO_BASE
    early_uart_init x23, x24, 0
    PRINT("- UART enabled -\r\n")
    ret

/* Print early debug messages.
 * x0: Nul-terminated string to print.
 * x23: Early UART base physical address
 * Clobbers x0-x1 */
puts:
    early_uart_ready x23, 1
    ldrb  w1, [x0], #1           /* Load next char */
    cbz   w1, 1f                 /* Exit on nul */
    early_uart_transmit x23, w1
    b     puts
1:
    ret

/*
 * Print a 64-bit number in hex.
 * x0: Number to print.
 * x23: Early UART base address
 * Clobbers x0-x3
 */
putn:
    adr   x1, hex
    mov   x3, #16
1:
    early_uart_ready x23, 2
    and   x2, x0, #(0xf<<60)     /* Mask off the top nybble */
    lsr   x2, x2, #60
    ldrb  w2, [x1, x2]           /* Convert to a char */
    early_uart_transmit x23, w2
    lsl   x0, x0, #4             /* Roll it through one nybble at a time */
    subs  x3, x3, #1
    b.ne  1b
    ret

hex:    .ascii "0123456789abcdef"
        .align 2

fail:   PRINT("- Boot failed -\r\n")
1:      wfe
        b     1b