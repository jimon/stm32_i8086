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

	if(i8086_init() != 0)
		ili9340_puts("failed to init 8086\n");

	int a = 0;
	while(1)
	{
		//ili9340_fill_color(ILI9340_BLUE);
		//ili9340_fill_color(ILI9340_RED);

		//ili9340_putc('0' + rand()%10);

		//char temp[128];
		//sprintf(temp, "%i\n", a);
		//ili9340_puts(temp);
		a++;

		//BSP_LED_Toggle(LED3);
		//BSP_LED_Toggle(LED4);
		//BSP_LED_Toggle(LED5);
		//BSP_LED_Toggle(LED6);
		HAL_Delay(5);
	}
}
