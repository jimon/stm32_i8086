#include "main.h"

#include "stm32_ili9340.h"

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	ili9340_init();
	ili9340_fill_color(ILI9340_BLUE);

	//for(int i = 0; i < 10; ++i)
	//	ili9340_putc('0' + i, 50, 100 - 6 * i);

	//ili9340_puts("HELLO WORLD\nTEST 123\nTEST TEST");

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);

	while(1)
	{
		//ili9340_fill_color(ILI9340_BLUE);
		//ili9340_fill_color(ILI9340_RED);

		ili9340_putc('0' + rand()%10);

		BSP_LED_Toggle(LED3);
		//BSP_LED_Toggle(LED4);
		//BSP_LED_Toggle(LED5);
		//BSP_LED_Toggle(LED6);
		//HAL_Delay(10);
	}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
	while(1)
	{
	}
}
#endif
