#include "stm32_i8086.h"
#include "stm32f4xx_hal.h"
#include "stm32_ili9340.h"
#include <stdbool.h>

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

void i8086_wreset(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wnmi(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wintr(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wready(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wtest(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_whold(bool enabled)	{HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wclk(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wdata8(uint8_t data)
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
void i8086_wdata16(uint16_t data)
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

bool i8086_rinta()		{return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET;}
bool i8086_rale()		{return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7) == GPIO_PIN_SET;}
bool i8086_rden()		{return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == GPIO_PIN_SET;}
bool i8086_rdtr()		{return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5) == GPIO_PIN_SET;}
bool i8086_rmio()		{return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4) == GPIO_PIN_SET;}
bool i8086_rwr()		{return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_3) == GPIO_PIN_SET;}
bool i8086_rhlda()		{return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == GPIO_PIN_SET;}
bool i8086_rrd()		{return HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0) == GPIO_PIN_SET;}
bool i8086_rbhe()		{return HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET;}
uint32_t i8086_raddr()
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

uint32_t tick_delay = 10;
void i8086_wclk_redge() {i8086_wclk(true);}
void i8086_wclk_up_wait() {HAL_Delay(tick_delay);}
void i8086_wclk_fedge() {i8086_wclk(false);}
void i8086_wclk_down_wait() {HAL_Delay(tick_delay * 2);}

void i8086_wclk_ticks(uint32_t ticks)
{
	for(uint32_t i = 0; i < ticks; ++i)
	{
		i8086_wclk_redge();
		i8086_wclk_up_wait();
		i8086_wclk_fedge();
		i8086_wclk_down_wait();
	}
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
	i8086_init_out_b.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
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
	i8086_init_in_d.Pin = GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	i8086_init_in_d.Mode = GPIO_MODE_INPUT;
	i8086_init_in_d.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOD, &i8086_init_in_d);

	i8086_wreset(true);
	i8086_wnmi(false);
	i8086_wintr(false);
	i8086_wready(true);
	i8086_wtest(false);
	i8086_whold(false);
	i8086_wclk(false);

	i8086_wclk_ticks(10); // should be 4, but whatever

	i8086_wreset(false);

	return 0;
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

void i8086_poll()
{
	while(true)
	{
		i8086_wclk_redge();
		i8086_wclk_up_wait();
		i8086_wclk_fedge();
		i8086_wclk_down_wait();

	}
}
