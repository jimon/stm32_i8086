#include "stm32_i8086.h"
#include "stm32f4xx_hal.h"
#include "stm32_ili9340.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// pin assignment :
// A00-A07	PA00-PA07 (in/out)
// A08-A09	PC04-PC05 (in/out)
// A10-A12	PB00-PB02 (in/out)
// A13-A15	PE07-PE09 (in/out)
// A16-A19	PE10-PE13 (in)
// INTA		PB03 (in)
// ALE		PD07 (in)
// DEN		PD06 (in)
// DT/R		PD05 (in)
// M/IO		PD04 (in)
// WR		PD03 (in)
// HLDA		PD02 (in)
// RD		PD00 (in)
// BHE		PE14 (in)
// INTR		PB08 (out)
// NMI		PB07 (out)
// RESET	PB06 (out)
// READY	PB05 (out)
// TEST		PB04 (out)
// HOLD		PD01 (out)
// CLK		PB09 (out)

// ----------------------------------------------- millisecond delay

#ifdef CLK_PROPER_DUTY_CYCLE
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
#endif

// ----------------------------------------------- generic write helpers

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

// ----------------------------------------------- generic read helpers

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
uint16_t i8086_rdata_0_7() {return i8086_raddr() & 0xff;}
uint16_t i8086_rdata_8_15() {return i8086_raddr() & 0xff00;}
uint16_t i8086_rdata_0_15() {return i8086_raddr() & 0xffff;}

// ----------------------------------------------- clock

void i8086_wclk_redge() {i8086_wclk(true);}
void i8086_wclk_fedge() {i8086_wclk(false);}
#ifdef CLK_PROPER_DUTY_CYCLE
uint32_t tick_delay = 1;
void i8086_wclk_up_wait() {delay_us(tick_delay);}
void i8086_wclk_down_wait() {delay_us(tick_delay * 2);}
#else
void i8086_wclk_up_wait() {}
void i8086_wclk_down_wait() {}
#endif

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

// ----------------------------------------------- reset

void i8086_reset()
{
	i8086_wreset(true);
	i8086_wclk_ticks(10); // should be >= 4 cycles
	i8086_wreset(false);
	// reset procedure will take ~10 cycles (7 on my cpu)
}

// ----------------------------------------------- bus direction

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

// ----------------------------------------------- init

int i8086_init()
{
	HAL_Delay(100); // wait 50+ ms after power up

	#ifdef CLK_PROPER_DUTY_CYCLE
	dwt_init();
	#endif

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

	i8086_wnmi(false);
	i8086_wintr(false);
	i8086_wready(true);
	i8086_wtest(true);
	i8086_whold(false);
	i8086_wclk(false);

	i8086_reset();

	return 0;
}

// ----------------------------------------------- debug helpers

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
	bool wr;
} history_op;
history_op history[16];
uint32_t history_ptr = 0;
void i8086_debug_stop(const char * s, uint32_t waits)
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
		sprintf(temp, "addr%x bhe%i rd%i wd%i\n",
				history[i].addr,
				history[i].bhe,
				history[i].rd,
				history[i].wr
				);
		ili9340_puts(temp);
	}
	while(true)
	{
	}
}

// ----------------------------------------------- memory simulation

uint8_t code_segment[0x8000];
uint8_t data_segment[0x8000];

uint8_t reset_vector[] =
{
	0xea, // direct intersegment jmp
	0x00, // offset low
	0x00, // offset high
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

void i8086_load_code_segment(const uint8_t * data, uint32_t size)
{
	memcpy(code_segment, data, size);
}

uint16_t memory_read(uint32_t addr)
{
	uint16_t data = 0;
	if(addr >= 0xffff0)
		data = *(uint16_t*)(reset_vector + (addr - 0xffff0));
	else if((addr >= 0x10000) && (addr <= 0x17fff))
		data = *(uint16_t*)(code_segment + (addr - 0x10000));
	else if((addr >= 0x20000) && (addr <= 0x27fff))
		data = *(uint16_t*)(data_segment + (addr - 0x20000));
	else
		data = 0xf4; // halt
	return data;
}

void memory_write(uint32_t addr, uint16_t data, uint16_t mask)
{
	uint16_t * ptr = 0;
	if((addr >= 0x10000) && (addr <= 0x17fff))
		ptr = (uint16_t*)(code_segment + (addr - 0x10000));
	else if((addr >= 0x20000) && (addr <= 0x27fff))
		ptr = (uint16_t*)(data_segment + (addr - 0x20000));
	else
		i8086_debug_stop("wrond write addr", 0);
	*ptr = data & mask + (*ptr) & ~mask;
}

// ----------------------------------------------- polling loop

void i8086_poll()
{
	// T1, T2, T3, T4 state
	uint8_t cycle = 1;

	// latches
	uint32_t latched_addr = 0;
	bool latched_bhe = false;
	bool rd = false;
	bool wd = false;

	// debug
	uint32_t debug_ale_waits = 0;
	uint32_t debug_rdwr_waits = 0;
	uint32_t debug_rdwr2_waits = 0;
	uint32_t debug_counter = 0;

	i8086_bus_read();

	while(true)
	{
		debug_counter++;

		if(debug_counter > 10000)
			i8086_debug_stop("works", 0);

		i8086_wnmi(false);
		i8086_wintr(false);
		i8086_wready(true);
		i8086_wtest(true);
		i8086_whold(false);
		i8086_wreset(false);

		switch(cycle)
		{
		case 1: // T1, wait for ale
		{
			i8086_wclk_down_wait();

			if(i8086_rale())
			{
				latched_addr = i8086_raddr();
				latched_bhe = i8086_rbhe();

				history[history_ptr].addr = latched_addr;
				history[history_ptr].bhe = latched_bhe;

				cycle = 2;
				debug_rdwr_waits = 0;
			}
			else
			{
				debug_ale_waits++;

				if(debug_ale_waits >= 100)
					i8086_debug_stop("T1: ale waits", debug_ale_waits);
			}

			i8086_wclk_redge();
			i8086_wclk_up_wait();
			i8086_wclk_fedge();
			break;
		}
		case 2: // T2, reads RD or WD, no waiting allowed
		{
			i8086_wclk_down_wait();
			i8086_wclk_redge();

			rd = false;
			wd = false;

			if(!i8086_rrd())
			{
				rd = true;
				cycle = 3;
				history[history_ptr].rd = true;
				history[history_ptr].wr = false;

				if(debug_rdwr_waits)
					i8086_debug_stop("T2: rdwr waits", debug_rdwr_waits);
			}
			else if(!i8086_rwr())
			{
				wd = true;
				cycle = 3;
				history[history_ptr].rd = false;
				history[history_ptr].wr = true;

				if(debug_rdwr_waits)
					i8086_debug_stop("T2: rdwr waits", debug_rdwr_waits);
			}
			else
			{
				debug_rdwr_waits++;
				if(debug_rdwr_waits >= 100)
					i8086_debug_stop("T2: rdwr waits", debug_rdwr_waits);
			}

			i8086_wclk_up_wait();
			i8086_wclk_fedge();
			break;
		}
		case 3:
		{
			if(rd)
			{
				i8086_bus_write(); // bus will switch back to read at the next cycle

				i8086_wclk_down_wait();

				uint8_t a0 = latched_addr & 1;
				uint32_t addr = latched_addr & ~1;
				uint16_t data = memory_read(addr);

				if((latched_bhe == 0) && (a0 == 0))
					i8086_wdata_0_15(data);
				else if((latched_bhe == 0) && (a0 == 1))
					i8086_wdata_8_15(data);
				else if((latched_bhe == 1) && (a0 == 0))
					i8086_wdata_0_7(data);
				else if((latched_bhe == 1) && (a0 == 1))
				{
					// do nothing
				}

				i8086_wclk_redge();
				i8086_wclk_up_wait();
				i8086_wclk_fedge();

				cycle = 4;
				debug_rdwr2_waits = 0;
			}

			if(wd)
			{
				i8086_wclk_down_wait();

				uint8_t a0 = latched_addr & 1;
				uint32_t addr = latched_addr & ~1;

				if((latched_bhe == 0) && (a0 == 0))
					memory_write(addr, i8086_rdata_0_15(), 0xffff);
				else if((latched_bhe == 0) && (a0 == 1))
					memory_write(addr, i8086_rdata_8_15(), 0xff00);
				else if((latched_bhe == 1) && (a0 == 0))
					memory_write(addr, i8086_rdata_0_7(),  0x00ff);
				else if((latched_bhe == 1) && (a0 == 1))
				{
					// do nothing
				}

				i8086_wclk_redge();
				i8086_wclk_up_wait();
				i8086_wclk_fedge();

				cycle = 4;
				debug_rdwr2_waits = 0;
			}

			break;
		}
		case 4:
		{
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
						i8086_debug_stop("T4: rdwr2 waits", debug_rdwr2_waits);
				}
				else
				{
					debug_rdwr2_waits++;
					if(debug_rdwr2_waits >= 100)
						i8086_debug_stop("T2: rdwr2 waits", debug_rdwr2_waits);
				}
			}
			else if(wd)
			{
				if(i8086_rwr())
				{
					history_ptr = (history_ptr + 1) % 16;
					cycle = 1;
					debug_ale_waits = 0;

					if(debug_rdwr2_waits)
						i8086_debug_stop("T4: rdwr2 waits", debug_rdwr2_waits);
				}
				else
				{
					debug_rdwr2_waits++;
					if(debug_rdwr2_waits >= 100)
						i8086_debug_stop("T2: rdwr2 waits", debug_rdwr2_waits);
				}
			}
			else
				i8086_debug_stop("T4: ?", 0);

			i8086_wclk_fedge();
			break;
		}
		default:
			break;
		}
	}
}
