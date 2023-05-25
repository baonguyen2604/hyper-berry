#include <stddef.h>
#include <stdint.h>

#ifndef MMIO_H
#define MMIO_H

// Loop <delay> times in a way that the compiler won't optimize away
void delay(int64_t count);

// Memory-Mapped I/O output
void mmio_write(uint64_t reg, uint64_t data);

// Memory-Mapped I/O input
uint64_t mmio_read(uint64_t reg);

#endif