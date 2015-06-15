#include "stm32_i8086.h"
#include "stm32f4xx_hal.h"

/*
A00-A07		PA00-PA07
A08-A09		PC04-PC05
A10-A12		PB00-PB02
A13-A19		PE07-PE13
PIN34		PE14
PIN32-PIN25	PD00-PD07
PIN24-PIN21	PB03-PB06
PIN17-PIN19	PB07-PB09

out
INTR		PB08
NMI			PB07
RESET		PB06
READY		PB05
TEST		PB04
HOLD		PD01

out clock
CLK			PB09

in
INTA		PB03
ALE			PD07
DEN			PD06
DT/R		PD05
M/IO		PD04
WR			PD03
HLDA		PD02
RD			PD00
BHE			PE14
*/

#include <stdbool.h>

TIM_HandleTypeDef clk_tim_handle;

void i8086_reset(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_nmi(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_intr(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_ready(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_test(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_hold(bool enabled)	{HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
bool i8086_bhe() {return HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET;}

uint32_t i8086_addr()
{
	return	  ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) << 0)
			| ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET) << 1)
			| ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_SET) << 2)
			| ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET) << 3)
			| ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) << 4)
			| ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) << 5)
			| ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET) << 6)
			| ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET) << 7)
			| ((HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4) == GPIO_PIN_SET) << 8)
			| ((HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5) == GPIO_PIN_SET) << 9)
			| ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) << 10)
			| ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) << 11)
			| ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET) << 12)
			| ((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_7) == GPIO_PIN_SET) << 13)
			| ((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == GPIO_PIN_SET) << 14)
			| ((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_9) == GPIO_PIN_SET) << 15)
			| ((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == GPIO_PIN_SET) << 16)
			| ((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11) == GPIO_PIN_SET) << 17)
			| ((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET) << 18)
			| ((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13) == GPIO_PIN_SET) << 19)
			;
}

void i8086_data8(uint8_t data)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, ((data >> 0) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, ((data >> 1) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, ((data >> 2) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, ((data >> 3) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, ((data >> 4) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, ((data >> 5) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, ((data >> 6) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, ((data >> 7) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void i8086_data16(uint16_t data)
{
	i8086_data8(data);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, ((data >>  8) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, ((data >>  9) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, ((data >> 10) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, ((data >> 11) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, ((data >> 12) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, ((data >> 13) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, ((data >> 14) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, ((data >> 15) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void i8086_bus_read()
{
	GPIO_InitTypeDef i8086_init_bus;
	i8086_init_bus.Mode = GPIO_MODE_INPUT;
	i8086_init_bus.Speed = GPIO_SPEED_HIGH;
	i8086_init_bus.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	HAL_GPIO_Init(GPIOA, &i8086_init_bus);
	i8086_init_bus.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	HAL_GPIO_Init(GPIOC, &i8086_init_bus);
	i8086_init_bus.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
	HAL_GPIO_Init(GPIOB, &i8086_init_bus);
	i8086_init_bus.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	HAL_GPIO_Init(GPIOE, &i8086_init_bus);
}

void i8086_bus_write()
{
	GPIO_InitTypeDef i8086_init_bus;
	i8086_init_bus.Mode = GPIO_MODE_OUTPUT_PP;
	i8086_init_bus.Pull = GPIO_PULLUP;
	i8086_init_bus.Speed = GPIO_SPEED_HIGH;

	i8086_init_bus.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	HAL_GPIO_Init(GPIOA, &i8086_init_bus);
	i8086_init_bus.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	HAL_GPIO_Init(GPIOC, &i8086_init_bus);
	i8086_init_bus.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
	HAL_GPIO_Init(GPIOB, &i8086_init_bus);
	i8086_init_bus.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	HAL_GPIO_Init(GPIOE, &i8086_init_bus);
}


#include "stm32_ili9340.h"

int i8086_init()
{
	HAL_Delay(50);

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	i8086_bus_read();

	GPIO_InitTypeDef i8086_init_addr_e;
	i8086_init_addr_e.Pin =  GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
	i8086_init_addr_e.Mode = GPIO_MODE_INPUT;
	i8086_init_addr_e.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOE, &i8086_init_addr_e);

	GPIO_InitTypeDef i8086_init_out_b;
	i8086_init_out_b.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
	i8086_init_out_b.Mode = GPIO_MODE_OUTPUT_PP;
	i8086_init_out_b.Pull = GPIO_PULLUP;
	i8086_init_out_b.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &i8086_init_out_b);

	GPIO_InitTypeDef i8086_init_out_d;
	i8086_init_out_d.Pin = GPIO_PIN_1;
	i8086_init_out_d.Mode = GPIO_MODE_OUTPUT_PP;
	i8086_init_out_d.Pull = GPIO_PULLUP;
	i8086_init_out_d.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOD, &i8086_init_out_d);

	GPIO_InitTypeDef i8086_init_in_b;
	i8086_init_in_b.Pin = GPIO_PIN_3;
	i8086_init_in_b.Mode = GPIO_MODE_INPUT;
	i8086_init_in_b.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &i8086_init_in_b);

	GPIO_InitTypeDef i8086_init_in_d;
	i8086_init_in_d.Pin = GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5;// | GPIO_PIN_6;
	i8086_init_in_d.Mode = GPIO_MODE_INPUT;
	i8086_init_in_d.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOD, &i8086_init_in_d);

	i8086_reset(true);
	i8086_nmi(false);
	i8086_intr(false);
	i8086_ready(true);
	i8086_test(false);
	i8086_hold(false);

	GPIO_InitTypeDef clk_output_init;
	clk_output_init.Pin = GPIO_PIN_9;
	clk_output_init.Mode = GPIO_MODE_AF_PP;
	clk_output_init.Pull = GPIO_PULLUP;
	clk_output_init.Speed = GPIO_SPEED_HIGH;
	clk_output_init.Alternate = GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOB, &clk_output_init);
	__HAL_RCC_TIM4_CLK_ENABLE();
	clk_tim_handle.Instance = TIM4;
	clk_tim_handle.Init.Period = 15;
	clk_tim_handle.Init.Prescaler = 50000;
	clk_tim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	clk_tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	clk_tim_handle.Init.RepetitionCounter = 0;
	TIM_OC_InitTypeDef tim_output_init;
	tim_output_init.OCMode = TIM_OCMODE_PWM1;
	tim_output_init.Pulse = 5;
	tim_output_init.OCFastMode = TIM_OCFAST_ENABLE;
	tim_output_init.OCPolarity = TIM_OCPOLARITY_HIGH;
	HAL_TIM_PWM_Init(&clk_tim_handle);
	HAL_TIM_PWM_ConfigChannel(&clk_tim_handle, &tim_output_init, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start_IT(&clk_tim_handle, TIM_CHANNEL_4);

	HAL_Delay(50);

	/*GPIO_InitTypeDef i8086_init_in_d_it;
	i8086_init_in_d_it.Mode = GPIO_MODE_IT_RISING;
	i8086_init_in_d_it.Pull = GPIO_NOPULL;
	i8086_init_in_d_it.Pin = GPIO_PIN_7 | GPIO_PIN_6;
	i8086_init_in_d_it.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOD, &i8086_init_in_d_it);
	i8086_init_in_d_it.Mode = GPIO_MODE_IT_FALLING;
	i8086_init_in_d_it.Pull = GPIO_NOPULL;
	i8086_init_in_d_it.Pin = GPIO_PIN_0 | GPIO_PIN_3;
	i8086_init_in_d_it.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOD, &i8086_init_in_d_it);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);*/

	HAL_Delay(50);
	i8086_reset(false);

	return 0;
}

void i8086_poll()
{
}

/*
void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
	HAL_TIM_IRQHandler(&clk_tim_handle);
}

void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}

uint32_t latched_addr = 0;

uint8_t reset_vector[] =
{
	0b11101010, // JMP Direct Intersegment
	0x0, // offset low
	0x0, // offset high
	0x80, // seg low
	0x0,  // seg high

	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,

};

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
	return;

	switch(pin)
	{
	case GPIO_PIN_0:
	{
		if(latched_addr >= 0xffff0) // in reset vector
		{
			//bool bhe = i8086_bhe();

			uint16_t val1 = reset_vector[latched_addr - 0xffff0];
			uint16_t val2 = reset_vector[latched_addr - 0xffff0 + 1];
			char temp[128];
			sprintf(temp, "irq: rd in reset vector = 0x%x 0x%x\n", val1, val2);
			ili9340_puts(temp);


			i8086_bus_write();
			i8086_data16(val2 << 8 + val1);

			i8086_ready(true);
		}
		else
		{
			ili9340_puts("irq: rd unknown\n");
			i8086_hold(true);
		}


		break;
	}
	case GPIO_PIN_3:
		ili9340_puts("irq: wd\n");
		break;
	case GPIO_PIN_6:
		//i8086_bus_read();
		ili9340_puts("irq: den end\n");
		break;
	case GPIO_PIN_7:
	{
		i8086_bus_read();
		latched_addr = i8086_addr();
		i8086_ready(false);
		char temp[128];
		sprintf(temp, "irq: ale (0x%x)\n", latched_addr);
		ili9340_puts(temp);
		break;
	}
	default:
		ili9340_puts("irq: ?\n");
		break;
	}
}
*/

