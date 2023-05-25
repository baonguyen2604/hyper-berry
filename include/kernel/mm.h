#include <kernel/compiler.h>

#ifndef __ASSEMBLY__

#include <kernel/types.h>
#include <stdint.h>
typedef uint64_t pgtbl_t;

#endif

#ifndef _MM_H
#define _MM_H

#define KB(_kb)     (_AC(_kb, ULL) << 10)
#define MB(_mb)     (_AC(_mb, ULL) << 20)
#define GB(_gb)     (_AC(_gb, ULL) << 30)

/*
 * Memory layout
 * 0	-	512K 	Unmapped
 * 512K -	2M		Text, data, bss (from linker script?)
 * 2M	-	6M		Fixmap
 */

#define L0_ENTRY_BITS  39
#define L0_INDEX(slot) (_AT(vaddr_t,slot) << L0_ENTRY_BITS)
#define L0_ENTRY_SIZE  L0(1)

#define HYP_VIRT_START 			(L0_INDEX(1) + _AT(vaddr_t, MB(2)) + _AT(vaddr_t, KB(512)))
#define HYP_VIRT_SIZE			_AT(vaddr_t, MB(2))

#define FIXMAP_VIRT_START		(HYP_VIRT_START + HYP_VIRT_SIZE)
#define FIXMAP_VIRT_SIZE		_AT(vaddr_t, MB(2))

/* Fixmap slots */
#define FIXMAP_UART  0 /* UART1 / Mini UART */

#define FIXMAP_ADDR(page)		(FIXMAP_VIRT_START + (page) * PAGE_SIZE)

#define EARLY_UART1_VIRTUAL_ADDRESS \
	(FIXMAP_ADDR(FIXMAP_UART) + (AUX_BASE & ~PAGE_MASK))

#define STACK_SIZE				PAGE_SIZE

/*
 * Memory attribute
 * G: Gathering
 * R: Re-ordering
 * E: Early Write Ack
 * 
 * 						AttrIdx		encoding
 * MT_DEVICE_nGnRnE		000			0000 0000		-- Strongly Ordered Device nGnRnE
 * MT_DEVICE_nGnRE		001			0000 0100		-- Device nGnRE
 * MT_NORMAL			010			1111 1111		-- Normal write-back write-allocate (TODO: read about RW-allocate)
 * reserved				011
 * reserved				100
 * reserved				101
 * reserved				110
 * reserved				111
 *
 * Attribute Indexes.
 *
 * These are valid in the AttrIndx[2:0] field of a stage 1 page
 * table entry. They are indexes into the bytes of the MAIR*
 * registers
 *
 */
#define MT_DEVICE_nGnRnE 	0x0
#define MT_DEVICE_nGnRE     0x1
#define MT_NORMAL     		0x2

#define _MAIR(attr, mt) (attr << (mt) * 8)

#define MAIRVAL (_MAIR(0x00, MT_DEVICE_nGnRnE) | \
				 _MAIR(0x04, MT_DEVICE_nGnRE) | \
				 _MAIR(0xff, MT_NORMAL))

/* Translation table configuration
 * Only support 4k page granularity now 
 */
#define PAGE_SHIFT              12
#define PAGE_SIZE				(1 << PAGE_SHIFT)
#define PAGE_MASK           	(~(PAGE_SIZE-1))
#define PT_LEVEL_BITS			(PAGE_SHIFT - 3)
#define PT_ENTRIES				(1 << PT_LEVEL_BITS)
#define PT_ENTRY_MASK			(PT_ENTRIES - 1)
#define PT_LEVEL_SHIFT(lvl)		(PAGE_SHIFT + PT_LEVEL_BITS * (3 - lvl))
#define PT_LEVEL_SIZE(lvl)		(1 << PT_LEVEL_SHIFT(lvl))
#define PT_LEVEL_MASK(lvl)		(~(PT_LEVEL_SIZE(lvl) - 1))

/* Convenience macros
 */
#define PT_LEVEL_3_SHIFT		PAGE_SHIFT
#define PT_LEVEL_2_SHIFT		(PT_LEVEL_3_SHIFT + PT_LEVEL_BITS)
#define PT_LEVEL_1_SHIFT		(PT_LEVEL_2_SHIFT + PT_LEVEL_BITS)
#define PT_LEVEL_0_SHIFT		(PT_LEVEL_1_SHIFT + PT_LEVEL_BITS)

#define PT_LEVEL_3_MASK			PT_LEVEL_MASK(3)
#define PT_LEVEL_2_MASK			PT_LEVEL_MASK(2)
#define PT_LEVEL_1_MASK			PT_LEVEL_MASK(1)
#define PT_LEVEL_0_MASK			PT_LEVEL_MASK(0)

/* Calculate the offsets into the pagetables for a given VA */
#define level_3_linear_offset(va) ((va) >> PT_LEVEL_3_SHIFT)
#define level_2_linear_offset(va) ((va) >> PT_LEVEL_2_SHIFT)
#define level_1_linear_offset(va) ((va) >> PT_LEVEL_1_SHIFT)
#define level_0_linear_offset(va) ((va) >> PT_LEVEL_0_SHIFT)

#define TABLE_OFFSET(offset) (offset & PT_ENTRY_MASK)
#define level_3_table_offset(va)	TABLE_OFFSET(level_3_linear_offset(va))
#define level_2_table_offset(va)	TABLE_OFFSET(level_2_linear_offset(va))
#define level_1_table_offset(va)	TABLE_OFFSET(level_1_linear_offset(va))
#define level_0_table_offset(va)	TABLE_OFFSET(level_0_linear_offset(va))

#define DEFINE_BOOT_PAGE_TABLE(name)                         \
pgtbl_t __aligned(PAGE_SIZE) __section(".data.page_aligned") \
    name[PT_ENTRIES]

#endif

// 0000 0080 0020 0000

// 0000 0000 0000 0000 | 0000 0000 1 | 000 0000 00 | 00 0000 001 | 0 1000 0000 | 0000 0000 0000
//                     | 1           | 0           | 1           | 128

// 0000 0200 0020 0040

// 0000 0000 0000 0000 | 0000 0010 0 | 000 0000 00 | 00 0000 001 | 0 0000 0000 | 0000 0100 0000
//                     | L0 index    | L1 index    | L2 index    | L3 index    | 
//                     | 4           | 0           | 1           | 0

// 1 1 11 01 1 010 1 1
// 1 1 11 01 1 010 0 1
// 1 1 11 01 1 010 1 1
// 1 1 10 01 1 001 0 1
// 1 1 10 01 1 001 1 1

// 0000 0200 0020 a000

// 0000 0000 0000 0000 | 0000 0010 0 | 000 0000 00 | 00 0000 001 | 0 0000 1010 | 0000 0000 0000
//                     | L0 index    | L1 index    | L2 index    | L3 index    | 
//                     | 4           | 0           | 1           | a