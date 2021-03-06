#include "stm32_ili9340.h"
#include "stm32f4xx_hal.h"

/*
some code here is based on
https://github.com/adafruit/Adafruit_ILI9340
*/

#define ILI9340_TFTWIDTH  240
#define ILI9340_TFTHEIGHT 320

#define ILI9340_NOP     0x00
#define ILI9340_SWRESET 0x01
#define ILI9340_RDDID   0x04
#define ILI9340_RDDST   0x09

#define ILI9340_SLPIN   0x10
#define ILI9340_SLPOUT  0x11
#define ILI9340_PTLON   0x12
#define ILI9340_NORON   0x13

#define ILI9340_RDMODE  0x0A
#define ILI9340_RDMADCTL  0x0B
#define ILI9340_RDPIXFMT  0x0C
#define ILI9340_RDIMGFMT  0x0A
#define ILI9340_RDSELFDIAG  0x0F

#define ILI9340_INVOFF  0x20
#define ILI9340_INVON   0x21
#define ILI9340_GAMMASET 0x26
#define ILI9340_DISPOFF 0x28
#define ILI9340_DISPON  0x29

#define ILI9340_CASET   0x2A
#define ILI9340_PASET   0x2B
#define ILI9340_RAMWR   0x2C
#define ILI9340_RAMRD   0x2E

#define ILI9340_PTLAR   0x30
#define ILI9340_MADCTL  0x36

#define ILI9340_MADCTL_MY  0x80
#define ILI9340_MADCTL_MX  0x40
#define ILI9340_MADCTL_MV  0x20
#define ILI9340_MADCTL_ML  0x10
#define ILI9340_MADCTL_RGB 0x00
#define ILI9340_MADCTL_BGR 0x08
#define ILI9340_MADCTL_MH  0x04

#define ILI9340_PIXFMT  0x3A

#define ILI9340_FRMCTR1 0xB1
#define ILI9340_FRMCTR2 0xB2
#define ILI9340_FRMCTR3 0xB3
#define ILI9340_INVCTR  0xB4
#define ILI9340_DFUNCTR 0xB6

#define ILI9340_PWCTR1  0xC0
#define ILI9340_PWCTR2  0xC1
#define ILI9340_PWCTR3  0xC2
#define ILI9340_PWCTR4  0xC3
#define ILI9340_PWCTR5  0xC4
#define ILI9340_VMCTR1  0xC5
#define ILI9340_VMCTR2  0xC7

#define ILI9340_RDID1   0xDA
#define ILI9340_RDID2   0xDB
#define ILI9340_RDID3   0xDC
#define ILI9340_RDID4   0xDD

#define ILI9340_GMCTRP1 0xE0
#define ILI9340_GMCTRN1 0xE1

SPI_HandleTypeDef ili9340_spi;
uint16_t ili9340_width = ILI9340_TFTWIDTH;
uint16_t ili9340_height = ILI9340_TFTHEIGHT;
uint16_t ili9340_cursor_x = 0;
uint16_t ili9340_cursor_y = 0;

int ili9340_wc(char command)
{
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_DC, GPIO_PIN_RESET);
	return (HAL_SPI_Transmit(&ili9340_spi, (uint8_t*)&command, 1, 1000) != HAL_OK) ? -1 : 0;
}

int ili9340_wd_impl(uint8_t data[], int size)
{
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_DC, GPIO_PIN_SET);
	return (HAL_SPI_Transmit(&ili9340_spi, data, size, 1000) != HAL_OK) ? -1 : 0;
}
#define ili9340_wd(...) \
	{ \
		uint8_t _temp[] = { __VA_ARGS__ }; \
		ili9340_wd_impl(_temp, sizeof(_temp) / sizeof(uint8_t)); \
	}
int ili9340_wd1(uint8_t data)
{
	return ili9340_wd_impl(&data, 1);
}

void ili9340_set_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	ili9340_wc(ILI9340_CASET);
	ili9340_wd(x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF);
	ili9340_wc(ILI9340_PASET);
	ili9340_wd(y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF);
	ili9340_wc(ILI9340_RAMWR);
}

int ili9340_fill_color(uint16_t color)
{
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_RESET);
	ili9340_set_rect(0, 0, ili9340_width, ili9340_height);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_DC, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_RESET);

	uint16_t slot[64];
	for(int i = 0; i < 64; ++i)
		slot[i] = ((color & 0xff) << 8) | (color >> 8);

	uint32_t bytes_left = ili9340_width * ili9340_height * 2;

	while(bytes_left)
	{
		uint32_t size = sizeof(slot) >= bytes_left ? bytes_left : sizeof(slot);
		bytes_left -= size;

		if(HAL_SPI_Transmit(&ili9340_spi, (uint8_t*)slot, size, 1000) != HAL_OK)
		{
			return -1;
		}
	}

	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_SET);
	return 0;
}

int ili9340_bitmap(uint8_t * data, uint16_t data_size, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_RESET);
	ili9340_set_rect(x, y, x + w - 1, y + h - 1);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_DC, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_RESET);

	if(HAL_SPI_Transmit(&ili9340_spi, data, data_size, 1000) != HAL_OK)
	{
		return -1;
	}

	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_SET);
	return 0;
}

int ili9340_bitmap_masked(uint8_t * data, uint16_t data_size, uint16_t mask_color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	for(int i = 0; i < h; ++i)
	{
		for(int j = 0; j < w; ++j)
		{
			uint16_t pixel = ((uint16_t*)data)[i * w + j];

			if(pixel == mask_color)
				continue;

			HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_RESET);
			ili9340_set_rect(x + j, y + i, x + j + 1, y + i + 1);
			HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_DC, GPIO_PIN_SET);
			HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_RESET);
			if(HAL_SPI_Transmit(&ili9340_spi, (uint8_t*)&pixel, 2, 1000) != HAL_OK)
			{
				return -1;
			}

			HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_SET);
		}
	}

	return 0;
}

// adafruit 5x7 font
uint8_t font[] = {
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
	0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
	0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
	0x18, 0x3C, 0x7E, 0x3C, 0x18,
	0x1C, 0x57, 0x7D, 0x57, 0x1C,
	0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
	0x00, 0x18, 0x3C, 0x18, 0x00,
	0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
	0x00, 0x18, 0x24, 0x18, 0x00,
	0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
	0x30, 0x48, 0x3A, 0x06, 0x0E,
	0x26, 0x29, 0x79, 0x29, 0x26,
	0x40, 0x7F, 0x05, 0x05, 0x07,
	0x40, 0x7F, 0x05, 0x25, 0x3F,
	0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
	0x7F, 0x3E, 0x1C, 0x1C, 0x08,
	0x08, 0x1C, 0x1C, 0x3E, 0x7F,
	0x14, 0x22, 0x7F, 0x22, 0x14,
	0x5F, 0x5F, 0x00, 0x5F, 0x5F,
	0x06, 0x09, 0x7F, 0x01, 0x7F,
	0x00, 0x66, 0x89, 0x95, 0x6A,
	0x60, 0x60, 0x60, 0x60, 0x60,
	0x94, 0xA2, 0xFF, 0xA2, 0x94,
	0x08, 0x04, 0x7E, 0x04, 0x08,
	0x10, 0x20, 0x7E, 0x20, 0x10,
	0x08, 0x08, 0x2A, 0x1C, 0x08,
	0x08, 0x1C, 0x2A, 0x08, 0x08,
	0x1E, 0x10, 0x10, 0x10, 0x10,
	0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
	0x30, 0x38, 0x3E, 0x38, 0x30,
	0x06, 0x0E, 0x3E, 0x0E, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x5F, 0x00, 0x00,
	0x00, 0x07, 0x00, 0x07, 0x00,
	0x14, 0x7F, 0x14, 0x7F, 0x14,
	0x24, 0x2A, 0x7F, 0x2A, 0x12,
	0x23, 0x13, 0x08, 0x64, 0x62,
	0x36, 0x49, 0x56, 0x20, 0x50,
	0x00, 0x08, 0x07, 0x03, 0x00,
	0x00, 0x1C, 0x22, 0x41, 0x00,
	0x00, 0x41, 0x22, 0x1C, 0x00,
	0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
	0x08, 0x08, 0x3E, 0x08, 0x08,
	0x00, 0x80, 0x70, 0x30, 0x00,
	0x08, 0x08, 0x08, 0x08, 0x08,
	0x00, 0x00, 0x60, 0x60, 0x00,
	0x20, 0x10, 0x08, 0x04, 0x02,
	0x3E, 0x51, 0x49, 0x45, 0x3E,
	0x00, 0x42, 0x7F, 0x40, 0x00,
	0x72, 0x49, 0x49, 0x49, 0x46,
	0x21, 0x41, 0x49, 0x4D, 0x33,
	0x18, 0x14, 0x12, 0x7F, 0x10,
	0x27, 0x45, 0x45, 0x45, 0x39,
	0x3C, 0x4A, 0x49, 0x49, 0x31,
	0x41, 0x21, 0x11, 0x09, 0x07,
	0x36, 0x49, 0x49, 0x49, 0x36,
	0x46, 0x49, 0x49, 0x29, 0x1E,
	0x00, 0x00, 0x14, 0x00, 0x00,
	0x00, 0x40, 0x34, 0x00, 0x00,
	0x00, 0x08, 0x14, 0x22, 0x41,
	0x14, 0x14, 0x14, 0x14, 0x14,
	0x00, 0x41, 0x22, 0x14, 0x08,
	0x02, 0x01, 0x59, 0x09, 0x06,
	0x3E, 0x41, 0x5D, 0x59, 0x4E,
	0x7C, 0x12, 0x11, 0x12, 0x7C,
	0x7F, 0x49, 0x49, 0x49, 0x36,
	0x3E, 0x41, 0x41, 0x41, 0x22,
	0x7F, 0x41, 0x41, 0x41, 0x3E,
	0x7F, 0x49, 0x49, 0x49, 0x41,
	0x7F, 0x09, 0x09, 0x09, 0x01,
	0x3E, 0x41, 0x41, 0x51, 0x73,
	0x7F, 0x08, 0x08, 0x08, 0x7F,
	0x00, 0x41, 0x7F, 0x41, 0x00,
	0x20, 0x40, 0x41, 0x3F, 0x01,
	0x7F, 0x08, 0x14, 0x22, 0x41,
	0x7F, 0x40, 0x40, 0x40, 0x40,
	0x7F, 0x02, 0x1C, 0x02, 0x7F,
	0x7F, 0x04, 0x08, 0x10, 0x7F,
	0x3E, 0x41, 0x41, 0x41, 0x3E,
	0x7F, 0x09, 0x09, 0x09, 0x06,
	0x3E, 0x41, 0x51, 0x21, 0x5E,
	0x7F, 0x09, 0x19, 0x29, 0x46,
	0x26, 0x49, 0x49, 0x49, 0x32,
	0x03, 0x01, 0x7F, 0x01, 0x03,
	0x3F, 0x40, 0x40, 0x40, 0x3F,
	0x1F, 0x20, 0x40, 0x20, 0x1F,
	0x3F, 0x40, 0x38, 0x40, 0x3F,
	0x63, 0x14, 0x08, 0x14, 0x63,
	0x03, 0x04, 0x78, 0x04, 0x03,
	0x61, 0x59, 0x49, 0x4D, 0x43,
	0x00, 0x7F, 0x41, 0x41, 0x41,
	0x02, 0x04, 0x08, 0x10, 0x20,
	0x00, 0x41, 0x41, 0x41, 0x7F,
	0x04, 0x02, 0x01, 0x02, 0x04,
	0x40, 0x40, 0x40, 0x40, 0x40,
	0x00, 0x03, 0x07, 0x08, 0x00,
	0x20, 0x54, 0x54, 0x78, 0x40,
	0x7F, 0x28, 0x44, 0x44, 0x38,
	0x38, 0x44, 0x44, 0x44, 0x28,
	0x38, 0x44, 0x44, 0x28, 0x7F,
	0x38, 0x54, 0x54, 0x54, 0x18,
	0x00, 0x08, 0x7E, 0x09, 0x02,
	0x18, 0xA4, 0xA4, 0x9C, 0x78,
	0x7F, 0x08, 0x04, 0x04, 0x78,
	0x00, 0x44, 0x7D, 0x40, 0x00,
	0x20, 0x40, 0x40, 0x3D, 0x00,
	0x7F, 0x10, 0x28, 0x44, 0x00,
	0x00, 0x41, 0x7F, 0x40, 0x00,
	0x7C, 0x04, 0x78, 0x04, 0x78,
	0x7C, 0x08, 0x04, 0x04, 0x78,
	0x38, 0x44, 0x44, 0x44, 0x38,
	0xFC, 0x18, 0x24, 0x24, 0x18,
	0x18, 0x24, 0x24, 0x18, 0xFC,
	0x7C, 0x08, 0x04, 0x04, 0x08,
	0x48, 0x54, 0x54, 0x54, 0x24,
	0x04, 0x04, 0x3F, 0x44, 0x24,
	0x3C, 0x40, 0x40, 0x20, 0x7C,
	0x1C, 0x20, 0x40, 0x20, 0x1C,
	0x3C, 0x40, 0x30, 0x40, 0x3C,
	0x44, 0x28, 0x10, 0x28, 0x44,
	0x4C, 0x90, 0x90, 0x90, 0x7C,
	0x44, 0x64, 0x54, 0x4C, 0x44,
	0x00, 0x08, 0x36, 0x41, 0x00,
	0x00, 0x00, 0x77, 0x00, 0x00,
	0x00, 0x41, 0x36, 0x08, 0x00,
	0x02, 0x01, 0x02, 0x04, 0x02,
	0x3C, 0x26, 0x23, 0x26, 0x3C,
	0x1E, 0xA1, 0xA1, 0x61, 0x12,
	0x3A, 0x40, 0x40, 0x20, 0x7A,
	0x38, 0x54, 0x54, 0x55, 0x59,
	0x21, 0x55, 0x55, 0x79, 0x41,
	0x22, 0x54, 0x54, 0x78, 0x42, // a-umlaut
	0x21, 0x55, 0x54, 0x78, 0x40,
	0x20, 0x54, 0x55, 0x79, 0x40,
	0x0C, 0x1E, 0x52, 0x72, 0x12,
	0x39, 0x55, 0x55, 0x55, 0x59,
	0x39, 0x54, 0x54, 0x54, 0x59,
	0x39, 0x55, 0x54, 0x54, 0x58,
	0x00, 0x00, 0x45, 0x7C, 0x41,
	0x00, 0x02, 0x45, 0x7D, 0x42,
	0x00, 0x01, 0x45, 0x7C, 0x40,
	0x7D, 0x12, 0x11, 0x12, 0x7D, // A-umlaut
	0xF0, 0x28, 0x25, 0x28, 0xF0,
	0x7C, 0x54, 0x55, 0x45, 0x00,
	0x20, 0x54, 0x54, 0x7C, 0x54,
	0x7C, 0x0A, 0x09, 0x7F, 0x49,
	0x32, 0x49, 0x49, 0x49, 0x32,
	0x3A, 0x44, 0x44, 0x44, 0x3A, // o-umlaut
	0x32, 0x4A, 0x48, 0x48, 0x30,
	0x3A, 0x41, 0x41, 0x21, 0x7A,
	0x3A, 0x42, 0x40, 0x20, 0x78,
	0x00, 0x9D, 0xA0, 0xA0, 0x7D,
	0x3D, 0x42, 0x42, 0x42, 0x3D, // O-umlaut
	0x3D, 0x40, 0x40, 0x40, 0x3D,
	0x3C, 0x24, 0xFF, 0x24, 0x24,
	0x48, 0x7E, 0x49, 0x43, 0x66,
	0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
	0xFF, 0x09, 0x29, 0xF6, 0x20,
	0xC0, 0x88, 0x7E, 0x09, 0x03,
	0x20, 0x54, 0x54, 0x79, 0x41,
	0x00, 0x00, 0x44, 0x7D, 0x41,
	0x30, 0x48, 0x48, 0x4A, 0x32,
	0x38, 0x40, 0x40, 0x22, 0x7A,
	0x00, 0x7A, 0x0A, 0x0A, 0x72,
	0x7D, 0x0D, 0x19, 0x31, 0x7D,
	0x26, 0x29, 0x29, 0x2F, 0x28,
	0x26, 0x29, 0x29, 0x29, 0x26,
	0x30, 0x48, 0x4D, 0x40, 0x20,
	0x38, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x38,
	0x2F, 0x10, 0xC8, 0xAC, 0xBA,
	0x2F, 0x10, 0x28, 0x34, 0xFA,
	0x00, 0x00, 0x7B, 0x00, 0x00,
	0x08, 0x14, 0x2A, 0x14, 0x22,
	0x22, 0x14, 0x2A, 0x14, 0x08,
	0xAA, 0x00, 0x55, 0x00, 0xAA,
	0xAA, 0x55, 0xAA, 0x55, 0xAA,
	0x00, 0x00, 0x00, 0xFF, 0x00,
	0x10, 0x10, 0x10, 0xFF, 0x00,
	0x14, 0x14, 0x14, 0xFF, 0x00,
	0x10, 0x10, 0xFF, 0x00, 0xFF,
	0x10, 0x10, 0xF0, 0x10, 0xF0,
	0x14, 0x14, 0x14, 0xFC, 0x00,
	0x14, 0x14, 0xF7, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0xFF,
	0x14, 0x14, 0xF4, 0x04, 0xFC,
	0x14, 0x14, 0x17, 0x10, 0x1F,
	0x10, 0x10, 0x1F, 0x10, 0x1F,
	0x14, 0x14, 0x14, 0x1F, 0x00,
	0x10, 0x10, 0x10, 0xF0, 0x00,
	0x00, 0x00, 0x00, 0x1F, 0x10,
	0x10, 0x10, 0x10, 0x1F, 0x10,
	0x10, 0x10, 0x10, 0xF0, 0x10,
	0x00, 0x00, 0x00, 0xFF, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0xFF, 0x10,
	0x00, 0x00, 0x00, 0xFF, 0x14,
	0x00, 0x00, 0xFF, 0x00, 0xFF,
	0x00, 0x00, 0x1F, 0x10, 0x17,
	0x00, 0x00, 0xFC, 0x04, 0xF4,
	0x14, 0x14, 0x17, 0x10, 0x17,
	0x14, 0x14, 0xF4, 0x04, 0xF4,
	0x00, 0x00, 0xFF, 0x00, 0xF7,
	0x14, 0x14, 0x14, 0x14, 0x14,
	0x14, 0x14, 0xF7, 0x00, 0xF7,
	0x14, 0x14, 0x14, 0x17, 0x14,
	0x10, 0x10, 0x1F, 0x10, 0x1F,
	0x14, 0x14, 0x14, 0xF4, 0x14,
	0x10, 0x10, 0xF0, 0x10, 0xF0,
	0x00, 0x00, 0x1F, 0x10, 0x1F,
	0x00, 0x00, 0x00, 0x1F, 0x14,
	0x00, 0x00, 0x00, 0xFC, 0x14,
	0x00, 0x00, 0xF0, 0x10, 0xF0,
	0x10, 0x10, 0xFF, 0x10, 0xFF,
	0x14, 0x14, 0x14, 0xFF, 0x14,
	0x10, 0x10, 0x10, 0x1F, 0x00,
	0x00, 0x00, 0x00, 0xF0, 0x10,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0xFF,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0x38, 0x44, 0x44, 0x38, 0x44,
	0xFC, 0x4A, 0x4A, 0x4A, 0x34, // sharp-s or beta
	0x7E, 0x02, 0x02, 0x06, 0x06,
	0x02, 0x7E, 0x02, 0x7E, 0x02,
	0x63, 0x55, 0x49, 0x41, 0x63,
	0x38, 0x44, 0x44, 0x3C, 0x04,
	0x40, 0x7E, 0x20, 0x1E, 0x20,
	0x06, 0x02, 0x7E, 0x02, 0x02,
	0x99, 0xA5, 0xE7, 0xA5, 0x99,
	0x1C, 0x2A, 0x49, 0x2A, 0x1C,
	0x4C, 0x72, 0x01, 0x72, 0x4C,
	0x30, 0x4A, 0x4D, 0x4D, 0x30,
	0x30, 0x48, 0x78, 0x48, 0x30,
	0xBC, 0x62, 0x5A, 0x46, 0x3D,
	0x3E, 0x49, 0x49, 0x49, 0x00,
	0x7E, 0x01, 0x01, 0x01, 0x7E,
	0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
	0x44, 0x44, 0x5F, 0x44, 0x44,
	0x40, 0x51, 0x4A, 0x44, 0x40,
	0x40, 0x44, 0x4A, 0x51, 0x40,
	0x00, 0x00, 0xFF, 0x01, 0x03,
	0xE0, 0x80, 0xFF, 0x00, 0x00,
	0x08, 0x08, 0x6B, 0x6B, 0x08,
	0x36, 0x12, 0x36, 0x24, 0x36,
	0x06, 0x0F, 0x09, 0x0F, 0x06,
	0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x10, 0x10, 0x00,
	0x30, 0x40, 0xFF, 0x01, 0x01,
	0x00, 0x1F, 0x01, 0x01, 0x1E,
	0x00, 0x19, 0x1D, 0x17, 0x12,
	0x00, 0x3C, 0x3C, 0x3C, 0x3C,
	0x00, 0x00, 0x00, 0x00, 0x00
};

uint16_t scroll_y = 0;

int ili9340_putc(char c)
{
	uint16_t slot[7 * 5];
	for(int i = 0; i < 7; ++i)
		for(int j = 0; j < 5; ++j)
			slot[i * 5 + j] = ((font[((uint8_t)c) * 5 + j] >> i) & 0x1) ? ILI9340_WHITE : ILI9340_BLACK;

	int res = 0;
	if(c != '\n')
		res = ili9340_bitmap((uint8_t*)slot, sizeof(slot), ili9340_cursor_x, ili9340_cursor_y, 5, 7);

	ili9340_cursor_x += 6;

	if((ili9340_cursor_x > ili9340_width - 5) || (c == '\n'))
	{
		ili9340_cursor_x = 0;
		ili9340_cursor_y += 8;

		if(ili9340_cursor_y >= ili9340_height - 7)
		{
			ili9340_cursor_x = 0;
			ili9340_cursor_y = 0;
		}

		for(int i = 0; i < 7; ++i)
			for(int j = 0; j < 5; ++j)
				slot[i * 5 + j] = ILI9340_BLACK;

		for(int x = ili9340_cursor_x; x < ili9340_width; x += 6)
			res = ili9340_bitmap((uint8_t*)slot, sizeof(slot), x, ili9340_cursor_y, 5, 7);
	}

	return res;
}

int ili9340_puts(char * str)
{
	if(!str)
		return 0;
	while(*str)
	{
		if(ili9340_putc(*str) != 0)
			return -1;
		str++;
	}
	return 0;
}

int ili9340_clr(uint16_t color)
{
	ili9340_cursor_x = 0;
	ili9340_cursor_y = 0;
	return ili9340_fill_color(color);
}

int ili9340_init(uint8_t rotation, uint8_t inverted)
{
	ILI9340_SPI_CLK_ENABLED();
	ILI9340_GPIO_CLK_ENABLED();

	GPIO_InitTypeDef  GPIO_InitStruct;
	GPIO_InitStruct.Pin       = GPIO_PIN_13;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_14;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	ili9340_spi.Instance				= ILI9340_SPI;
	ili9340_spi.Init.BaudRatePrescaler	= SPI_BAUDRATEPRESCALER_2;
	ili9340_spi.Init.Direction			= SPI_DIRECTION_2LINES;
	ili9340_spi.Init.CLKPhase			= SPI_PHASE_1EDGE;
	ili9340_spi.Init.CLKPolarity		= SPI_POLARITY_LOW;
	ili9340_spi.Init.CRCCalculation		= SPI_CRCCALCULATION_DISABLE;
	ili9340_spi.Init.CRCPolynomial		= 7;
	ili9340_spi.Init.DataSize			= SPI_DATASIZE_8BIT;
	ili9340_spi.Init.FirstBit			= SPI_FIRSTBIT_MSB;
	ili9340_spi.Init.NSS				= SPI_NSS_SOFT;
	ili9340_spi.Init.TIMode				= SPI_TIMODE_DISABLE;
	ili9340_spi.Init.Mode				= SPI_MODE_MASTER;

	if(HAL_SPI_Init(&ili9340_spi) != HAL_OK)
	{
		return -1;
	}

	GPIO_InitTypeDef ili9340_pins_init;
	ili9340_pins_init.Pin = ILI9340_CS | ILI9340_RS | ILI9340_DC;
	ili9340_pins_init.Mode = GPIO_MODE_OUTPUT_PP;
	ili9340_pins_init.Pull = GPIO_PULLUP;
	ili9340_pins_init.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(ILI9340_GPIO, &ili9340_pins_init);

	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_RS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_DC, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_RS, GPIO_PIN_SET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_RS, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_RS, GPIO_PIN_SET);
	HAL_Delay(150);

	ili9340_wc(0xEF);
	ili9340_wd(0x03, 0x80, 0x02);

	ili9340_wc(0xCF);
	ili9340_wd(0x00, 0xC1, 0x30);

	ili9340_wc(0xED);
	ili9340_wd(0x64, 0x03, 0x12, 0x81);

	ili9340_wc(0xE8);
	ili9340_wd(0x85, 0x00, 0x78);

	ili9340_wc(0xCB);
	ili9340_wd(0x39, 0x2C, 0x00, 0x34, 0x02);

	ili9340_wc(0xF7);
	ili9340_wd1(0x20);

	ili9340_wc(0xEA);
	ili9340_wd(0x00, 0x00);

	ili9340_wc(ILI9340_PWCTR1);
	ili9340_wd1(0x23);

	ili9340_wc(ILI9340_PWCTR2);
	ili9340_wd1(0x10);

	ili9340_wc(ILI9340_VMCTR1);
	ili9340_wd(0x3e, 0x28);

	ili9340_wc(ILI9340_VMCTR2);
	ili9340_wd1(0x86);

	ili9340_wc(ILI9340_MADCTL);
	ili9340_wd1(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);

	ili9340_wc(ILI9340_PIXFMT);
	ili9340_wd1(0x55);

	ili9340_wc(ILI9340_FRMCTR1);
	ili9340_wd(0x00, 0x18);

	ili9340_wc(ILI9340_DFUNCTR);
	ili9340_wd(0x08, 0x82, 0x27);

	ili9340_wc(0xF2);
	ili9340_wd1(0x00);

	ili9340_wc(ILI9340_GAMMASET);
	ili9340_wd1(0x01);

	ili9340_wc(ILI9340_GMCTRP1);
	ili9340_wd(0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00);

	ili9340_wc(ILI9340_GMCTRN1);
	ili9340_wd(0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F);

	ili9340_wc(ILI9340_SLPOUT);
	HAL_Delay(120);

	ili9340_wc(ILI9340_DISPON);

	ili9340_wc(ILI9340_MADCTL);
	switch(rotation % 4)
	{
	default:
	case 0:
		ili9340_wd1(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
		ili9340_width  = ILI9340_TFTWIDTH; ili9340_height = ILI9340_TFTHEIGHT;
		break;
	case 1:
		ili9340_wd1(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
		ili9340_width  = ILI9340_TFTHEIGHT; ili9340_height = ILI9340_TFTWIDTH;
		break;
	case 2:
		ili9340_wd1(ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR);
		ili9340_width  = ILI9340_TFTWIDTH; ili9340_height = ILI9340_TFTHEIGHT;
		break;
	case 3:
		ili9340_wd1(ILI9340_MADCTL_MV | ILI9340_MADCTL_MY | ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
		ili9340_width  = ILI9340_TFTHEIGHT; ili9340_height = ILI9340_TFTWIDTH;
		break;
	}

	ili9340_wc(inverted ? ILI9340_INVON : ILI9340_INVOFF);

	return 0;
}
