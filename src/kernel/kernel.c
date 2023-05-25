#include <stddef.h>
#include <stdint.h>
#include <kernel/compiler.h>
#include <kernel/uart.h>
#include <kernel/mini_uart.h>
#include <common/stdio.h>

// arguments for AArch64
// void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
void __init kernel_main(unsigned long boot_phys_offset,
                        unsigned long fdt_paddr)
{
    // mini_uart_init();
    mini_uart_puts("Hello, kernel World!\r\n");

    while (1) {
        uart_putc(uart_getc());
        // gets(buf,256);
        // puts(buf);
        // putc('\n');
    }
}