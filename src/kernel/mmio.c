#include <kernel/mmio.h>

void delay(int64_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

void mmio_write(uint64_t reg, uint64_t data)
{
    *(volatile uint64_t*)reg = data;
}

uint64_t mmio_read(uint64_t reg)
{
    return *(volatile uint64_t*)reg;
}