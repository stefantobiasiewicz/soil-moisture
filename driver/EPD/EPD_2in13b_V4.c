/*****************************************************************************
* | File      	:  	EPD_2IN13b_V4.c
* | Author      :   Waveshare team
* | Function    :   2.13inch e-paper B V4
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2022-04-25
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "EPD_2in13b_V4.h"

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_2IN13B_V4_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(20);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(2);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(20);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_2IN13B_V4_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_2IN13B_V4_SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
void EPD_2IN13B_V4_ReadBusy(void)
{
    // Debug("e-Paper busy\r\n");
	while(1)
	{	 //=1 BUSY
		if(DEV_Digital_Read(EPD_BUSY_PIN)==0) 
			break;
		DEV_Delay_ms(20);
	}
	DEV_Delay_ms(20);
    // Debug("e-Paper busy release\r\n");
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/

static void EPD_2IN13_V4_TurnOnDisplay(void)
{
	EPD_2IN13B_V4_SendCommand(0x20); // Activate Display Update Sequence
	EPD_2IN13B_V4_ReadBusy();
}

static void EPD_2IN13_V4_TurnOnDisplay_Fast(void)
{
	EPD_2IN13B_V4_SendCommand(0x22); // Display Update Control
	EPD_2IN13B_V4_SendData(0xc7);	// fast:0x0c, quality:0x0f, 0xcf
	EPD_2IN13B_V4_SendCommand(0x20); // Activate Display Update Sequence
	EPD_2IN13B_V4_ReadBusy();
}

static void EPD_2IN13_V4_TurnOnDisplay_Partial(void)
{
	EPD_2IN13B_V4_SendCommand(0x22); // Display Update Control
	EPD_2IN13B_V4_SendData(0xff);	// fast:0x0c, quality:0x0f, 0xcf
	EPD_2IN13B_V4_SendCommand(0x20); // Activate Display Update Sequence
	EPD_2IN13B_V4_ReadBusy();
}

/******************************************************************************
function :	Setting the display window
parameter:
******************************************************************************/
static void EPD_2IN13B_V4_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    EPD_2IN13B_V4_SendCommand(0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
    EPD_2IN13B_V4_SendData((Xstart>>3) & 0xFF);
    EPD_2IN13B_V4_SendData((Xend>>3) & 0xFF);
	
    EPD_2IN13B_V4_SendCommand(0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
    EPD_2IN13B_V4_SendData(Ystart & 0xFF);
    EPD_2IN13B_V4_SendData((Ystart >> 8) & 0xFF);
    EPD_2IN13B_V4_SendData(Yend & 0xFF);
    EPD_2IN13B_V4_SendData((Yend >> 8) & 0xFF);
}

/******************************************************************************
function :	Set Cursor
parameter:
******************************************************************************/
static void EPD_2IN13B_V4_SetCursor(UWORD Xstart, UWORD Ystart)
{
    EPD_2IN13B_V4_SendCommand(0x4E); // SET_RAM_X_ADDRESS_COUNTER
    EPD_2IN13B_V4_SendData(Xstart & 0xFF);

    EPD_2IN13B_V4_SendCommand(0x4F); // SET_RAM_Y_ADDRESS_COUNTER
    EPD_2IN13B_V4_SendData(Ystart & 0xFF);
    EPD_2IN13B_V4_SendData((Ystart >> 8) & 0xFF);
}


/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_2IN13B_V4_Init(void)
{
	EPD_2IN13B_V4_Reset();

	EPD_2IN13B_V4_ReadBusy();   
	EPD_2IN13B_V4_SendCommand(0x12);  //SWRESET
	EPD_2IN13B_V4_ReadBusy();   

	EPD_2IN13B_V4_SendCommand(0x01); //Driver output control      
	EPD_2IN13B_V4_SendData(0xf9);
	EPD_2IN13B_V4_SendData(0x00);
	EPD_2IN13B_V4_SendData(0x00);

	EPD_2IN13B_V4_SendCommand(0x11); //data entry mode       
	EPD_2IN13B_V4_SendData(0x03);

	EPD_2IN13B_V4_SetWindows(0, 0, EPD_2IN13B_V4_WIDTH-1, EPD_2IN13B_V4_HEIGHT-1);
	EPD_2IN13B_V4_SetCursor(0, 0);

	EPD_2IN13B_V4_SendCommand(0x3C); //BorderWavefrom
	EPD_2IN13B_V4_SendData(0x05);	

	EPD_2IN13B_V4_SendCommand(0x18); //Read built-in temperature sensor
	EPD_2IN13B_V4_SendData(0x80);	

	EPD_2IN13B_V4_SendCommand(0x21); //  Display update control
	EPD_2IN13B_V4_SendData(0x80);	
	EPD_2IN13B_V4_SendData(0x80);

	EPD_2IN13B_V4_ReadBusy();

}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_2IN13B_V4_Clear(void)
{
	UWORD Width, Height;
    Width = (EPD_2IN13B_V4_WIDTH % 8 == 0)? (EPD_2IN13B_V4_WIDTH / 8 ): (EPD_2IN13B_V4_WIDTH / 8 + 1);
    Height = EPD_2IN13B_V4_HEIGHT;
	
    EPD_2IN13B_V4_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2IN13B_V4_SendData(0XFF);
        }
    }	
    EPD_2IN13B_V4_SendCommand(0x26);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2IN13B_V4_SendData(0XFF);
        }
    }
	EPD_2IN13_V4_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_2IN13B_V4_Display(const UBYTE *blackImage, const UBYTE *redImage)
{
	UWORD Width, Height;
    Width = (EPD_2IN13B_V4_WIDTH % 8 == 0)? (EPD_2IN13B_V4_WIDTH / 8 ): (EPD_2IN13B_V4_WIDTH / 8 + 1);
    Height = EPD_2IN13B_V4_HEIGHT;
	
    EPD_2IN13B_V4_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2IN13B_V4_SendData(blackImage[i + j * Width]);
        }
    }	
	EPD_2IN13B_V4_SendCommand(0x26);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2IN13B_V4_SendData(redImage[i + j * Width]);
        }
    }	
	EPD_2IN13_V4_TurnOnDisplay();	
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_2IN13B_V4_Sleep(void)
{
	EPD_2IN13B_V4_SendCommand(0x10); //enter deep sleep
	EPD_2IN13B_V4_SendData(0x01); 
	DEV_Delay_ms(100);
}




/**
 * 
 * test to implement zephyr device driver
 * 
 * 
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>


unsigned char gImage_2in13b_V4b[4000] = {0x00};
unsigned char gImage_2in13b_V4r[4000] = {0x00};



static int my_display_init(const struct device *dev) {
	memset(gImage_2in13b_V4b, 0xff, 4000);
	memset(gImage_2in13b_V4r, 0xff, 4000);

	return 0;
}

static int dummy_display_write(const struct device *dev, const uint16_t x,
			       const uint16_t y,
			       const struct display_buffer_descriptor *desc,
			       const void *buf)
{
	// const struct dummy_display_config *config = dev->config;

	// __ASSERT(desc->width <= desc->pitch, "Pitch is smaller then width");
	// __ASSERT(desc->pitch <= config->width,
	// 	"Pitch in descriptor is larger than screen size");
	// __ASSERT(desc->height <= config->height,
	// 	"Height in descriptor is larger than screen size");
	// __ASSERT(x + desc->pitch <= config->width,
	// 	 "Writing outside screen boundaries in horizontal direction");
	// __ASSERT(y + desc->height <= config->height,
	// 	 "Writing outside screen boundaries in vertical direction");


	
	// int y_bolck = (desc->width/8) * y;
	// for (int i = 0; i < desc->buf_size; i++) {
	// 	gImage_2in13b_V4b[y_bolck + i] = *(unsigned char*)&buf[i];
	// }

	int base_index = (desc->width/8) * y - 1;	
	for (int i = 0; i < desc->buf_size; i++) {
		uint8_t px = *(uint8_t*)&buf[i];
		uint8_t px_bit = i % 8;

		if(px_bit == 0) {
			base_index ++;
		}
		if(px == 0x01) {
			// set black
			gImage_2in13b_V4b[base_index] &= ~BIT(7 -px_bit);
			gImage_2in13b_V4r[base_index] |= BIT(7 -px_bit);
		} else if (px == 0x02) {
			// set red
			gImage_2in13b_V4r[base_index] &= ~BIT(7 -px_bit);
			gImage_2in13b_V4b[base_index] |= BIT(7 -px_bit);

		} else {
			//set white
			gImage_2in13b_V4b[base_index] |= BIT(7 - px_bit);
			gImage_2in13b_V4r[base_index] |= BIT(7 -px_bit);	
		}
	}


	return 0;
}

static int dummy_display_blanking_off(const struct device *dev)
{    
	/**
	 * function blanking off is call on end of zephyr-lvgl flush method when screen_info contain 'SCREEN_INFO_EPD' 
	 */
	EPD_2IN13B_V4_Display(gImage_2in13b_V4b, gImage_2in13b_V4r);
	return 0;
}

static int dummy_display_blanking_on(const struct device *dev)
{
	return 0;
}

static int dummy_display_set_brightness(const struct device *dev,
					const uint8_t brightness)
{
	return 0;
}

static int dummy_display_set_contrast(const struct device *dev,
				      const uint8_t contrast)
{
	return 0;
}

static void dummy_display_get_capabilities(const struct device *dev,
		struct display_capabilities *capabilities)
{
	capabilities->x_resolution = EPD_2IN13B_V4_WIDTH;
	capabilities->y_resolution = EPD_2IN13B_V4_HEIGHT;
	capabilities->supported_pixel_formats = PIXEL_FORMAT_RGB_565;
	capabilities->current_pixel_format = PIXEL_FORMAT_RGB_565;
	capabilities->screen_info = SCREEN_INFO_EPD | SCREEN_INFO_MONO_MSB_FIRST;
	capabilities->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

static int dummy_display_set_pixel_format(const struct device *dev,
		const enum display_pixel_format pixel_format)
{
	return 0;
}

static const struct display_driver_api my_display_api = {
	.blanking_on = dummy_display_blanking_on,
	.blanking_off = dummy_display_blanking_off,
	.write = dummy_display_write,
	.set_brightness = dummy_display_set_brightness,
	.set_contrast = dummy_display_set_contrast,
	.get_capabilities = dummy_display_get_capabilities,
	.set_pixel_format = dummy_display_set_pixel_format,
};

#define EINK_DEFINE(node_id)                                                                   \
	DEVICE_DT_DEFINE(node_id, my_display_init, NULL, NULL, NULL, POST_KERNEL, CONFIG_DISPLAY_INIT_PRIORITY, &my_display_api);

DT_FOREACH_STATUS_OKAY(opencoded_epd, EINK_DEFINE)