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
	ili9340_set_rect(0, 0, 240, 320);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_DC, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_RESET);

	uint32_t bytes_left = 320 * 240 * 2;
	uint32_t bytes_read = 0;

	uint16_t temp[240 * 136];
	for(int i = 0; i < 240 * 136; ++i)
	{
		uint16_t c = color;
		temp[i] = ((c & 0xff) << 8) | (c >> 8);
	}

	while(bytes_left)
	{
		uint32_t slot = sizeof(temp);
		uint32_t size = slot >= bytes_left ? bytes_left : slot;

		bytes_left -= size;
		bytes_read += size;

		if(HAL_SPI_Transmit(&ili9340_spi, (uint8_t*)temp, size, 10000) != HAL_OK)
		{
			return -1;
		}
	}

	HAL_GPIO_WritePin(ILI9340_GPIO, ILI9340_CS, GPIO_PIN_SET);
	return 0;
}

int ili9340_init()
{
	ILI9340_SPI_CLK_ENABLED();
	ILI9340_GPIO_CLK_ENABLED();

	ili9340_spi.Instance               = ILI9340_SPI;
	ili9340_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	ili9340_spi.Init.Direction         = SPI_DIRECTION_2LINES;
	ili9340_spi.Init.CLKPhase          = SPI_PHASE_1EDGE;
	ili9340_spi.Init.CLKPolarity       = SPI_POLARITY_LOW;
	ili9340_spi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	ili9340_spi.Init.CRCPolynomial     = 7;
	ili9340_spi.Init.DataSize          = SPI_DATASIZE_8BIT;
	ili9340_spi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	ili9340_spi.Init.NSS               = SPI_NSS_SOFT;
	ili9340_spi.Init.TIMode            = SPI_TIMODE_DISABLE;
	ili9340_spi.Init.Mode              = SPI_MODE_MASTER;

	if(HAL_SPI_Init(&ili9340_spi) != HAL_OK)
	{
		return -1;
	}

	GPIO_InitTypeDef ili9340_pins_init;
	ili9340_pins_init.Pin = ILI9340_CS | ILI9340_RS | ILI9340_DC;
	ili9340_pins_init.Mode = GPIO_MODE_OUTPUT_PP;
	ili9340_pins_init.Pull = GPIO_PULLUP;
	ili9340_pins_init.Speed = GPIO_SPEED_FAST;
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

	ili9340_wc(ILI9340_SLPOUT);    //Exit Sleep
	HAL_Delay(120);

	ili9340_wc(ILI9340_DISPON);    //Display on

	return 0;
}
