#include <stddef.h>
#include <stdint.h>
#include <kernel/mm.h>
#include <kernel/mini_uart.h>
#include <kernel/mmio.h>
#include <kernel/peripherals.h>

#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)

void mini_uart_init()
{
    mmio_write(AUX_ENABLES, 1); //enable UART1
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3); //8 bits
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_IIR_REG, 0xC6); //disable interrupts
    mmio_write(AUX_MU_BAUD_REG, 270); // 250Hz core_freq

    uint32_t selector = mmio_read(GPFSEL1);
    selector &= ~((7<<12) | (7<<15));                   // clean gpio14, gpio15
    selector |= (2<<12) | (2<<15);                      // set alt5 for gpio14, gpio15

    mmio_write(GPFSEL1, selector);

    mmio_write(GPPUD, 0);
    delay(150);

    // Enable pull down for pin 14,15 & delay for 150 cycles.
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    mmio_write(GPPUDCLK0, 0x00000000);

    mmio_write(AUX_MU_CNTL_REG, 3);
}

void mini_uart_putc(unsigned char c)
{
    while (!(mmio_read(AUX_MU_LSR_REG_OFFSET + EARLY_UART1_VIRTUAL_ADDRESS) & 0x20)) {}
    mmio_write(AUX_MU_IO_REG_OFFSET + EARLY_UART1_VIRTUAL_ADDRESS, c);
}

unsigned char mini_uart_getc()
{
	while (!(mmio_read(AUX_MU_LSR_REG_OFFSET + EARLY_UART1_VIRTUAL_ADDRESS) & 0x01)) {}
	return mmio_read(AUX_MU_IO_REG_OFFSET + EARLY_UART1_VIRTUAL_ADDRESS);
}

void mini_uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        mini_uart_putc((unsigned char)str[i]);
}