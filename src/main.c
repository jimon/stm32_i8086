#include "main.h"
#include "stm32f4_discovery.h"
#include "stm32_ili9340.h"
#include "stm32_i8086.h"
#include <stdio.h>

#include "../rom/test_write.h"
#include "../rom/test_io.h"

int main(void)
{
	HAL_Init();
	SystemClock_Config();

	ili9340_init(3, 0);
	ili9340_fill_color(0);
	ili9340_puts("i8086 + stm32f4 = \x03\n");

	if(i8086_init() != 0)
		ili9340_puts("failed to init 8086\n");

	//i8086_load_code_segment(test_write_bin, test_write_bin_size);
	i8086_load_code_segment(test_io_bin, test_io_bin_size);

	i8086_poll();
}
