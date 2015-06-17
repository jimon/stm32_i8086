#include "stm32_i8086.h"
#include "stm32f4xx_hal.h"
#include "stm32_ili9340.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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


extern uint32_t SystemCoreClock;
void dwt_init(void)
{
	if(!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
	{
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	}
}
uint32_t dwt_get(void)
{
	return DWT->CYCCNT;
}
__inline uint8_t dwt_compare(int32_t tp)
{
	return (((int32_t)dwt_get() - tp) < 0);
}
void delay_us(uint32_t us)
{
	int32_t tp = dwt_get() + us * (SystemCoreClock / 1000000);
	while(dwt_compare(tp)) {}
}

void i8086_wreset(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wnmi(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wintr(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wready(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wtest(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_whold(bool enabled)	{HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wclk(bool enabled)	{HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);}
void i8086_wdata_0_7(uint16_t data)
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
void i8086_wdata_8_15(uint16_t data)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, ((data >>  8) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, ((data >>  9) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, ((data >> 10) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, ((data >> 11) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, ((data >> 12) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, ((data >> 13) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, ((data >> 14) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, ((data >> 15) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void i8086_wdata_0_15(uint16_t data)
{
	i8086_wdata_0_7(data);
	i8086_wdata_8_15(data);
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

uint32_t tick_delay = 1;
void i8086_wclk_redge() {i8086_wclk(true);}
void i8086_wclk_up_wait() {delay_us(tick_delay);}
void i8086_wclk_fedge() {i8086_wclk(false);}
void i8086_wclk_down_wait() {delay_us(tick_delay * 2);}
void i8086_wclk_ticks(uint32_t ticks)
{
	for(uint32_t i = 0; i < ticks; ++i)
	{
		i8086_wclk_down_wait();
		i8086_wclk_redge();
		i8086_wclk_up_wait();
		i8086_wclk_fedge();
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
	HAL_Delay(100);
	dwt_init();

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

	//i8086_wclk_ticks(40); // should be 4, but whatever

	i8086_wnmi(false);
	i8086_wintr(false);
	i8086_wready(true);
	i8086_wtest(true);
	i8086_whold(false);
	i8086_wclk(false);
	i8086_wreset(true);

	i8086_wclk_ticks(10); // should be 4, but whatever

	i8086_wreset(false);

	i8086_wclk_ticks(5);

	return 0;
}

#include "../rom/test_write.h"

#define rom_org 0x0
#define rom_data test_write_bin
#define rom_size test_write_bin_size

uint8_t reset_vector[] =
{
	0xea,
	(rom_org >> 0) & 0xff, // offset low
	(rom_org >> 8) & 0xff, // offset high
	0x00, // segment low
	0x10, // segment high

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
	0x0
};

uint16_t memory_read(uint32_t addr)
{
	if(addr >= (0xffff0 >> 1))
		return *((uint16_t*)reset_vector + (addr - 0x7fff8));
	else if((addr >= (0x10000 >> 1)) && (addr < ((0x10000 + rom_size) >> 1) + 1))
		return *((uint16_t*)rom_data + (addr - (0x10000 >> 1)));
	else
		return 0xf4; // halt
}

#define kprintf(format, ...) \
{ \
	char temp[128]; \
	sprintf(temp, format, ##__VA_ARGS__); \
	ili9340_puts(temp); \
}

typedef struct
{
	uint32_t addr;
	bool bhe;
	bool rd;
} history_op;

history_op history[16];
uint32_t history_ptr = 0;

void i8086_debug(const char * s, uint32_t waits)
{
	char temp[128];
	sprintf(temp, "%s [%i] inta%i ale%i den%i dtr%i mio%i\nwr%i hlda%i rd%i bhe%i addr0x%x\n",
		s,
		waits,
		i8086_rinta(),
		i8086_rale(),
		i8086_rden(),
		i8086_rdtr(),
		i8086_rmio(),
		i8086_rwr(),
		i8086_rhlda(),
		i8086_rrd(),
		i8086_rbhe(),
		i8086_raddr());
	ili9340_puts(temp);
	for(uint32_t i = 0; i < 16; ++i)
	{
		sprintf(temp, "%x %i %i\n", history[i].addr, history[i].bhe, history[i].rd);
		ili9340_puts(temp);
	}
	while(true)
	{
		BSP_LED_Toggle(LED3);
		HAL_Delay(400);
		//i8086_wclk_ticks(1);
	}
}

void i8086_poll()
{
	uint8_t cycle = 1;

	uint32_t latched_addr = 0;
	bool latched_bhe = false;
	bool rd = false;

	uint32_t debug_ale_waits = 0;
	uint32_t debug_rdwr_waits = 0;
	uint32_t debug_rdwr2_waits = 0;

	i8086_bus_read();

	uint32_t debug_counter = 0;

	while(true)
	{
		debug_counter++;

		if(debug_counter > 10000)
			i8086_debug("hm", 0);

		i8086_wnmi(false);
		i8086_wintr(false);
		i8086_wready(true);
		i8086_wtest(true);
		i8086_whold(false);
		i8086_wreset(false);

		switch(cycle)
		{
		case 1: // T1, wait for ale
			i8086_wclk_down_wait();

			if(i8086_rale())
			{
				latched_addr = i8086_raddr();
				latched_bhe = i8086_rbhe();

				history[history_ptr].addr = latched_addr;
				history[history_ptr].bhe = latched_bhe;

				cycle = 2;
				debug_rdwr_waits = 0;

				//if(debug_ale_waits)
				//	i8086_debug("T1: ale waits", debug_ale_waits);
			}
			else
			{
				debug_ale_waits++;

				if(debug_ale_waits > 100)
					i8086_debug("T1: ale waits", debug_ale_waits);
			}

			i8086_wclk_redge();
			i8086_wclk_up_wait();
			i8086_wclk_fedge();
			break;
		case 2: // T2, wait for RD or WD
			i8086_wclk_down_wait();
			i8086_wclk_redge();

			rd = false;
			if(!i8086_rrd())
			{
				rd = true;
				cycle = 3;
				history[history_ptr].rd = true;

				if(debug_rdwr_waits)
					i8086_debug("T2: rdwr waits", debug_rdwr_waits);
			}
			else if(!i8086_rwr())
			{
				history[history_ptr].rd = false;
				i8086_debug("T2: wr?", 0);
			}
			else
			{
				debug_rdwr_waits++;
				if(debug_rdwr_waits > 100)
					i8086_debug("T2: rdwr waits", debug_rdwr_waits);

				//cycle = 1;
				//debug_ale_waits = 0;
			}

			i8086_wclk_up_wait();
			i8086_wclk_fedge();
			break;
		case 3:
			if(rd)
			{
				i8086_bus_write();

				i8086_wclk_down_wait();

				uint8_t a0 = latched_addr & 1;
				uint32_t addr = latched_addr >> 1;

				uint16_t data = memory_read(addr);

				if((latched_bhe == 0) && (a0 == 0))
					i8086_wdata_0_15(data);
				else if((latched_bhe == 0) && (a0 == 1))
					i8086_wdata_8_15(data);
				else if((latched_bhe == 1) && (a0 == 0))
					i8086_wdata_0_7(data);
				else if((latched_bhe == 1) && (a0 == 1))
				{
				}

				i8086_wclk_redge();
				i8086_wclk_up_wait();
				i8086_wclk_fedge();

				cycle = 4;
				debug_rdwr2_waits = 0;
			}
			break;
		case 4:
			i8086_wclk_down_wait();
			i8086_wclk_redge();
			i8086_wclk_up_wait();

			if(rd)
			{
				if(i8086_rrd())
				{
					i8086_bus_read();
					history_ptr = (history_ptr + 1) % 16;
					cycle = 1;
					debug_ale_waits = 0;

					if(debug_rdwr2_waits)
						i8086_debug("T4: rdwr2 waits", debug_rdwr2_waits);
				}
				else
				{
					debug_rdwr2_waits++;
					if(debug_rdwr2_waits > 100)
						i8086_debug("T2: rdwr2 waits", debug_rdwr2_waits);
				}
			}
			else
				i8086_debug("T4: ?", 0);

			i8086_wclk_fedge();
			break;
		default:
			break;
		}
	}
}
