#pragma once

#include <stdint.h>
#include "spi_conf.h"

#define ILI9340_SPI					SPIx
#define ILI9340_SPI_CLK_ENABLED		SPIx_CLK_ENABLE
#define ILI9340_GPIO				GPIOB
#define ILI9340_GPIO_CLK_ENABLED	__HAL_RCC_GPIOB_CLK_ENABLE
#define ILI9340_CS					GPIO_PIN_10
#define ILI9340_RS					GPIO_PIN_11
#define ILI9340_DC					GPIO_PIN_12

// Color definitions
#define	ILI9340_BLACK   0x0000
#define	ILI9340_BLUE    0x001F
#define	ILI9340_RED     0xF800
#define	ILI9340_GREEN   0x07E0
#define ILI9340_CYAN    0x07FF
#define ILI9340_MAGENTA 0xF81F
#define ILI9340_YELLOW  0xFFE0
#define ILI9340_WHITE   0xFFFF

int ili9340_fill_color(uint16_t color);
int ili9340_bitmap(uint8_t * data, uint16_t data_size, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
int ili9340_bitmap_masked(uint8_t * data, uint16_t data_size, uint16_t mask_color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

int ili9340_putc(char c);
int ili9340_puts(char * str);
int ili9340_clr(uint16_t color);

int ili9340_init(uint8_t rotation, uint8_t inverted);

