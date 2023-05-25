#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#define PERIPHERAL_BASE		(0x3F000000)

// ***************************************
// Absolute addresses
// ***************************************

/**
 * Auxiliary
 */
#define AUX_BASE 			(PERIPHERAL_BASE + 0x215000)
#define AUX_ENABLES 		(AUX_BASE + 0x04)
#define AUX_MU_IO_REG 		(AUX_BASE + 0x40)
#define AUX_MU_IER_REG 		(AUX_BASE + 0x44)
#define AUX_MU_IIR_REG 		(AUX_BASE + 0x48)
#define AUX_MU_LCR_REG 		(AUX_BASE + 0x4c)
#define AUX_MU_MCR_REG 		(AUX_BASE + 0x50)
#define AUX_MU_LSR_REG 		(AUX_BASE + 0x54)
#define AUX_MU_CNTL_REG 	(AUX_BASE + 0x60)
#define AUX_MU_BAUD_REG 	(AUX_BASE + 0x68)

/**
 * GPIO
 */
// The GPIO registers base address
#define GPIO_BASE  			(PERIPHERAL_BASE + 0x200000)
// Function selectors
#define GPFSEL0 			(GPIO_BASE)
#define GPFSEL1 			(GPIO_BASE + 0x04)
#define GPFSEL2 			(GPIO_BASE + 0x08)
#define GPFSEL3 			(GPIO_BASE + 0x0C)
#define GPFSEL4 			(GPIO_BASE + 0x10)
#define GPFSEL5 			(GPIO_BASE + 0x14)
// Set
#define GPSET0 				(GPIO_BASE + 0x1C)
#define GPSET1 				(GPIO_BASE + 0x20)
// Clear
#define GPCLR0 				(GPIO_BASE + 0x28)
// Level
#define GPLEV0 				(GPIO_BASE + 0x34)
#define GPLEV1 				(GPIO_BASE + 0x38)
#define GPEDS0 				(GPIO_BASE + 0x40)
#define GPEDS1 				(GPIO_BASE + 0x44)
#define GPHEN0 				(GPIO_BASE + 0x64)
#define GPHEN1 				(GPIO_BASE + 0x68)
// Controls actuation of pull up/down to ALL GPIO pins.
#define GPPUD 				(GPIO_BASE + 0x94)
// Controls actuation of pull up/down for specific GPIO pin.
#define GPPUDCLK0 			(GPIO_BASE + 0x98)
#define GPPUDCLK1 			(GPIO_BASE + 0x9C)

/**
 * PL011 UART
 */
// The base address for UART.
#define UART0_BASE   		(PERIPHERAL_BASE + 0x201000)
// Data register
#define UART0_DR     		(UART0_BASE + 0x00)
// Receive status register/error clear register
#define UART0_RSRECR 		(UART0_BASE + 0x04)
// Flag register
#define UART0_FR     		(UART0_BASE + 0x18)
// Integer baud rate
#define UART0_IBRD   		(UART0_BASE + 0x24)
// Fractional baud rate
#define UART0_FBRD   		(UART0_BASE + 0x28)
// Line control register
#define UART0_LCRH   		(UART0_BASE + 0x2C)
// Control register
#define UART0_CR     		(UART0_BASE + 0x30)
// Interrupt FIFO level select register
#define UART0_IFLS   		(UART0_BASE + 0x34)
// Interrupt Mask Set/Clear register
#define UART0_IMSC   		(UART0_BASE + 0x38)
// Raw interrupt status register
#define UART0_RIS    		(UART0_BASE + 0x3C)
// Masked interrupt status register
#define UART0_MIS    		(UART0_BASE + 0x40)
// Interrupt clear register
#define UART0_ICR    		(UART0_BASE + 0x44)
// DMA Control register
#define UART0_DMACR  		(UART0_BASE + 0x48)

// ***************************************
// Relative addresses from device's base address
// ***************************************

/**
 * Auxiliary, offset from AUX_BASE
 */
#define AUX_ENABLES_OFFSET 			0x04
#define AUX_MU_IO_REG_OFFSET 		0x40
#define AUX_MU_IER_REG_OFFSET 		0x44
#define AUX_MU_IIR_REG_OFFSET 		0x48
#define AUX_MU_LCR_REG_OFFSET 		0x4c
#define AUX_MU_MCR_REG_OFFSET 		0x50
#define AUX_MU_LSR_REG_OFFSET 		0x54
#define AUX_MU_CNTL_REG_OFFSET 		0x60
#define AUX_MU_BAUD_REG_OFFSET 		0x68

/**
 * GPIO, offset from GPIO_BASE
 */
#define GPFSEL0_OFFSET 				0x00
#define GPFSEL1_OFFSET 				0x04
#define GPFSEL2_OFFSET 				0x08
#define GPFSEL3_OFFSET 				0x0C
#define GPFSEL4_OFFSET 				0x10
#define GPFSEL5_OFFSET 				0x14
#define GPSET0_OFFSET 				0x1C
#define GPSET1_OFFSET 				0x20
#define GPCLR0_OFFSET 				0x28
#define GPLEV0_OFFSET 				0x34
#define GPLEV1_OFFSET 				0x38
#define GPEDS0_OFFSET 				0x40
#define GPEDS1_OFFSET 				0x44
#define GPHEN0_OFFSET 				0x64
#define GPHEN1_OFFSET 				0x68
#define GPPUD_OFFSET 				0x94
#define GPPUDCLK0_OFFSET 			0x98
#define GPPUDCLK1_OFFSET 			0x9C

#endif