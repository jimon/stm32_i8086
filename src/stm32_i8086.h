#pragma once

#include <stdint.h>

// simulation memory map :
// code segment : 0x10000 - 0x17fff
// data segment : 0x20000 - 0x27fff
// reset vector : 0xffff0 - 0xfffff

#define CLK_PROPER_DUTY_CYCLE // simulate proper 33% duty cycle with milliseconds delay

int i8086_init();
void i8086_load_code_segment(const uint8_t * data, uint32_t size);
void i8086_poll();
