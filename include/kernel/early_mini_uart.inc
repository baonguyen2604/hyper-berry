#include "kernel/peripherals.h"

delay:
	subs	x0, x0, #1
	b.ne 	delay
	ret

/*
 * UART1 initialization
 * uart_addr: Mini UART base address
 * gpio_addr: GPIO base address
 * c: scratch register number
 */
.macro early_uart_init uart_addr, gpio_addr, c
		mov  x1, lr
		mov  w\c, #1
		str  w\c, [\uart_addr, #AUX_ENABLES_OFFSET]

		mov  w\c, #0
		str  w\c, [\uart_addr, #AUX_MU_CNTL_REG_OFFSET]

		mov  w\c, #3
		str  w\c, [\uart_addr, #AUX_MU_LCR_REG_OFFSET]

		mov  x\c, #0
		str  w\c, [\uart_addr, #AUX_MU_MCR_REG_OFFSET]

		mov  x\c, #0
		str  w\c, [\uart_addr, #AUX_MU_IER_REG_OFFSET]

		mov  x\c, #0xC6
		str  w\c, [\uart_addr, #AUX_MU_IIR_REG_OFFSET]

		mov  x\c, #270
		str  w\c, [\uart_addr, #AUX_MU_BAUD_REG_OFFSET]
		
		mov  x\c, #0x12
		bfi  x\c, x\c, #12, #6
		str  w\c, [\gpio_addr, #GPFSEL1_OFFSET]

		mov  w\c, #0
		str  w\c, [\gpio_addr, #GPPUD_OFFSET]

		mov  w\c, #150
		bl   delay

		mov  w\c, #0xC000
		str  w\c, [\gpio_addr, #GPPUDCLK0_OFFSET]

		mov  x\c, #150
		bl   delay

		mov  w\c, #0
		str  w\c, [\gpio_addr, #GPPUDCLK0_OFFSET]

		mov  x\c, #3
		str  w\c, [\gpio_addr, AUX_MU_CNTL_REG_OFFSET]

		mov  lr, x1
.endm

/*
 * UART1 wait UART to be ready to transmit
 * xb: register which contains the early UART base physical address
 * c: scratch register number
 */
.macro early_uart_ready xb, c
1:
        ldr   w\c, [\xb, #AUX_MU_LSR_REG_OFFSET]        	 /* <- AUX_MU_LSR_REG */
        and   w\c, w\c, #0x20
        cbz   w\c, 1b                /* Wait for the UART to be ready */
.endm

/*
 * UART1 transmit character
 * xb: register which contains the the early UART base physical address
 * wt: register which contains the character to transmit
 */
.macro early_uart_transmit xb, wt
        str  \wt, [\xb, #AUX_MU_IO_REG_OFFSET]        /* -> AUX_MU_IO_REG */
.endm
