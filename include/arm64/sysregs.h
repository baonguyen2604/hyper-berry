#ifndef _SYSREGS_H
#define _SYSREGS_H

#define PSR_MODE_BIT  0x10 /* Set iff AArch32 */
#define PSR_MODE_EL3h 0x0d
#define PSR_MODE_EL3t 0x0c
#define PSR_MODE_EL2h 0x09
#define PSR_MODE_EL2t 0x08
#define PSR_MODE_EL1h 0x05
#define PSR_MODE_EL1t 0x04
#define PSR_MODE_EL0t 0x00

// ***************************************
// MPIDR_EL1, Multiprocessor Affinity Register, Page 2619 of AArch64-Reference-Manual.
// ***************************************

#define MPIDR_AFF0_SHIFT    (0)
#define MPIDR_AFF0_MASK     (0xff << MPIDR_AFF0_SHIFT)

// ***************************************
// SCTLR_EL1, System Control Register (EL1), Page 2654 of AArch64-Reference-Manual.
// ***************************************

#define SCTLR_RESERVED                  (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EE_LITTLE_ENDIAN          (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN         (0 << 24)
#define SCTLR_I_CACHE_DISABLED          (0 << 12)
#define SCTLR_I_CACHE_ENABLED           (1 << 12)
#define SCTLR_D_CACHE_DISABLED          (0 << 2)
#define SCTLR_D_CACHE_ENABLED           (1 << 2)
#define SCTLR_MMU_DISABLED              (0 << 0)
#define SCTLR_MMU_ENABLED               (1 << 0)

#define SCTLR_VALUE_MMU_DISABLED		(SCTLR_RESERVED | SCTLR_I_CACHE_ENABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

// ***************************************
// TCR_EL2, Translation Control Register (EL1), Page 2696 of AArch64-Reference-Manual.
// ***************************************

#define TCR_T0SZ_SHIFT  (0)
#define TCR_T1SZ_SHIFT  (16)
#define TCR_T0SZ(x)     ((x) << TCR_T0SZ_SHIFT)

/*
 * According to ARM DDI 0487B.a, TCR_EL1.{T0SZ,T1SZ} (AArch64, page D7-2480)
 * comprises 6 bits and TTBCR.{T0SZ,T1SZ} (AArch32, page G6-5204) comprises 3
 * bits following another 3 bits for RES0. Thus, the mask for both registers
 * should be 0x3f.
 */
#define TCR_SZ_MASK     (0x3f))

#define TCR_EPD0        (1 << 7)
#define TCR_EPD1        (1 << 23)

#define TCR_IRGN0_NC    (0 << 8)
#define TCR_IRGN0_WBWA  (1 << 8)
#define TCR_IRGN0_WT    (2 << 8)
#define TCR_IRGN0_WB    (3 << 8)

#define TCR_ORGN0_NC    (0 << 10)
#define TCR_ORGN0_WBWA  (1 << 10)
#define TCR_ORGN0_WT    (2 << 10)
#define TCR_ORGN0_WB    (3 << 10)

#define TCR_SH0_NS      (0 << 12)
#define TCR_SH0_OS      (2 << 12)
#define TCR_SH0_IS      (3 << 12)

#define TCR_PS(x)       ((x) << 16)
#define TCR_TBI         (1 << 20)

#define TCR_RES1        ((1 << 31) | (1 << 23))

// ***************************************
// HCR_EL2, Hypervisor Configuration Register (EL2), Page 2487 of AArch64-Reference-Manual.
// ***************************************

#define HCR_RW	    			(1 << 31)
#define HCR_DEFAULT_VALUE		HCR_RW

// ***************************************
// SCR_EL3, Secure Configuration Register (EL3), Page 2648 of AArch64-Reference-Manual.
// ***************************************

#define SCR_RESERVED	    		(3 << 4)
#define SCR_RW						(1 << 10)
#define SCR_NS						(1 << 0)
#define SCR_DEFAULT_VALUE	    	(SCR_RESERVED | SCR_RW | SCR_NS)

// ***************************************
// SPSR_EL3, Saved Program Status Register (EL3) Page 389 of AArch64-Reference-Manual.
// ***************************************

#define SPSR_MASK_ALL 			(7 << 6)
#define SPSR_EL1h				(5 << 0)
#define SPSR_EL2h				(9 << 0)
#define SPSR_DEFAULT_VALUE		(SPSR_MASK_ALL | SPSR_EL2h)

#endif