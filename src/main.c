#include "main.h"

#include "stm32_ili9340.h"
#include "stm32_i8086.h"
#include <stdio.h>

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);

	ili9340_init(3, 0);
	ili9340_fill_color(0);
	ili9340_puts("i8086 + stm32f4 = \x03\n");

	HAL_Delay(100);

	if(i8086_init() != 0)
		ili9340_puts("failed to init 8086\n");

	i8086_poll();
}
