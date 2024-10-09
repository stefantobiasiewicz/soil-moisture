#include "display.h"

LOG_MODULE_REGISTER(display, LOG_LEVEL_DBG);


static eink_1in9_pins_t pin_controll;

unsigned char VAR_Temperature=20; 

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_1in9_Reset(void)
{
    pin_controll.pin_reset_set(true);
    k_msleep(200);
    pin_controll.pin_reset_set(false);
    k_msleep(20);
    pin_controll.pin_reset_set(true);
    k_msleep(200);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
static void EPD_1in9_ReadBusy(void)
{
    LOG_INF("e-Paper busy");
    k_msleep(20);
	while(1)
	{	 //=1 BUSY;
		if(pin_controll.pin_busy_read()==false) 
			break;
        //todo timeout condition
		k_msleep(50);
	}
    k_msleep(20);
    LOG_INF("e-Paper busy release");
}

/*
# temperature measurement
# You are advised to periodically measure the temperature and modify the driver parameters
# If an external temperature sensor is available, use an external temperature sensor
*/
static void EPD_1in9_Temperature(void)
{
    uint8_t ret;
    uint8_t regs[] = {0x7E, 0x81, 0xB4}; // ??? todo check what it is????
    ret = i2c_write_dt(&eink_1in9_com, regs, sizeof(regs));
    if(ret != 0){
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr,regs[0]);
        return;
    }


    k_msleep(10);      

    uint8_t data[2] = {0xe7, 0x00};
	// Set default frame time
	if(VAR_Temperature<5)
		data[1] = 0x31; // 0x31  (49+1)*20ms=1000ms
	else if(VAR_Temperature<10)
		data[1] = 0x22; // 0x22  (34+1)*20ms=700ms
	else if(VAR_Temperature<15)
		data[1] = 0x18; // 0x18  (24+1)*20ms=500ms
	else if(VAR_Temperature<20)
		data[1] = 0x13; // 0x13  (19+1)*20ms=400ms
	else
		data[1] = 0x0e; // 0x0e  (14+1)*20ms=300ms

    ret = i2c_write_dt(&eink_1in9_com, data, sizeof(data));
    if(ret != 0){
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr,data[0]);
        return;
    }
}

/*
# dispalying screen
*/
static void EPD_1in9_Write_Screen( unsigned char *image)
{
    uint8_t ret;

    /*
    (0xAC); // Close the sleep
	(0x2B); // turn on the power
	(0x40); // Write RAM address
	(0xA9); // Turn on the first SRAM
	(0xA8); // Shut down the first SRAM
    */
    uint8_t regs[] = {0xAC, 0x2B, 0x40, 0xA9, 0xA8}; 
    ret = i2c_write_dt(&eink_1in9_com, regs, sizeof(regs));
    if(ret != 0){
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr, regs[0]);
        return;
    }

    uint8_t buff[16] = {0x00};
    memcpy(buff, image, 15);
    ret = i2c_write_dt(&eink_1in9_data, buff, sizeof(buff));
    if(ret != 0){
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_data.addr,regs[0]);
        return;
    }


    /*
    (0xAB); // Turn on the second SRAM
    (0xAA); // Shut down the second SRAM
    (0xAF); // display on
    */
    uint8_t regs_[] = {0xAB, 0xAA, 0xAF}; 
    ret = i2c_write_dt(&eink_1in9_com, regs_, sizeof(regs_));
    if(ret != 0){
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr,regs_[0]);
        return;
    }


	EPD_1in9_ReadBusy();
	//delay(2000);
	
    /*
    (0xAE); // display off
    (0x28); // HV OFF
    (0xAD); // sleep in
    */
    uint8_t regs_2[] = {0xAE, 0x28, 0xAD}; 
    ret = i2c_write_dt(&eink_1in9_com, regs_2, sizeof(regs_2));
    if(ret != 0){
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr,regs_2[0]);
        return;
    }
}


/*
# DU waveform white extinction diagram + black out diagram
# Bureau of brush waveform
*/
static void EPD_1in9_lut_DU_WB(void)
{
    uint8_t data[] = {0x82, 0x80, 0x00, 0xC0, 0x80, 0x80, 0x62};
    int ret = i2c_write_dt(&eink_1in9_com, data, sizeof(data));

    if (ret != 0) {
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr, data[0]);
        return;
    }
}

/*   
# GC waveform
# The brush waveform
*/
static void EPD_1in9_lut_GC(void)
{
    uint8_t data[] = {0x82, 0x20, 0x00, 0xA0, 0x80, 0x40, 0x63};
    int ret = i2c_write_dt(&eink_1in9_com, data, sizeof(data));

    if (ret != 0) {
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr, data[0]);
        return;
    }
}

/* 
# 5 waveform  better ghosting
# Boot waveform
*/
static void EPD_1in9_lut_5S(void)
{
    uint8_t data[] = {0x82, 0x28, 0x20, 0xA8, 0xA0, 0x50, 0x65};
    int ret = i2c_write_dt(&eink_1in9_com, data, sizeof(data));

    if (ret != 0) {
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr, data[0]);
        return;
    }
}



/**
 * 
 * my code for controling display
 * 
 */

static uint8_t digit_left[] = {0xbf, 0x00, 0xfd, 0xf5, 0x47, 0xf7, 0xff, 0x21, 0xff, 0xf7, 0x00};  
static uint8_t digit_right[] = {0x1f, 0x1f, 0x17, 0x1f, 0x1f, 0x1d, 0x1d, 0x1f, 0x1f, 0x1f, 0x00};  
static uint8_t eink_segments[15] = {0x00};  

static void updateTemperatureDisplay(float temperature) {
    uint8_t temperature_digits[4];
    temperature_digits[0] = (uint8_t)(temperature / 100) % 10;
    temperature_digits[1] = (uint8_t)(temperature / 10) % 10;
    temperature_digits[2] = (uint8_t)(temperature) % 10;
    temperature_digits[3] = (uint8_t)(temperature * 10) % 10;

    // Jeśli temperatura jest mniejsza niż 100 lub 10, wyświetl odpowiednie puste cyfry
    if (temperature < 100) { temperature_digits[0] = 10; }
    if (temperature < 10) { temperature_digits[1] = 10; }

    eink_segments[0] = digit_right[temperature_digits[0]];
    eink_segments[1] = digit_left[temperature_digits[1]];
    eink_segments[2] = digit_right[temperature_digits[1]];
    eink_segments[3] = digit_left[temperature_digits[2]];
    eink_segments[4] = digit_right[temperature_digits[2]] | 0b00100000; // Punkt dziesiętny
    eink_segments[11] = digit_left[temperature_digits[3]];
    eink_segments[12] = digit_right[temperature_digits[3]];

    eink_segments[13] = 0x05; // Symbol °C
}

static void updateHumidityDisplay(float humidity) {
    uint8_t humidity_digits[3];
    humidity_digits[0] = (uint8_t)(humidity / 10) % 10;
    humidity_digits[1] = (uint8_t)(humidity) % 10;
    humidity_digits[2] = (uint8_t)(humidity * 10) % 10;

    // Jeśli wilgotność jest mniejsza niż 10, ustaw odpowiednią cyfrę na "pustą"
    if (humidity < 10) { humidity_digits[0] = 10; }

    eink_segments[5] = digit_left[humidity_digits[0]];
    eink_segments[6] = digit_right[humidity_digits[0]];
    eink_segments[7] = digit_left[humidity_digits[1]];
    eink_segments[8] = digit_right[humidity_digits[1]] | 0b00100000; // Punkt dziesiętny
    eink_segments[9] = digit_left[humidity_digits[2]];
    eink_segments[10] = digit_right[humidity_digits[2]] | 0b00100000; // Symbol procenta
}



void display_init(eink_1in9_pins_t *eink_1in9_pins) {
    int err;
    err = is_pointer_null(eink_1in9_pins);
    if (err != ERROR_OK) {
        LOG_ERR("eink_1in9_pins is NULL.");
        return;
    }

    err = is_pointer_null(eink_1in9_pins->pin_busy_read);
    if (err != ERROR_OK) {
        LOG_ERR("eink_1in9_pins->pin_busy_read is NULL.");
        return;
    }
    err = is_pointer_null(eink_1in9_pins->pin_reset_set);
    if (err != ERROR_OK) {
        LOG_ERR("eink_1in9_pins->pin_reset_set is NULL.");
        return;
    }

    pin_controll.pin_busy_read = eink_1in9_pins->pin_busy_read;
    pin_controll.pin_reset_set = eink_1in9_pins->pin_reset_set;

    display_power_on();
}


void display_power_on() {
	EPD_1in9_Reset();
	k_msleep(100);

    uint8_t ret;
    uint8_t power_on[] = {0x2B};
    ret = i2c_write_dt(&eink_1in9_com, power_on, sizeof(power_on));
    if(ret != 0){
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr,power_on[0]);
        return;
    }

	k_msleep(10);

    uint8_t boost_tson[] = {0xA7, 0xE0};
    ret = i2c_write_dt(&eink_1in9_com, boost_tson, sizeof(boost_tson));
    if(ret != 0){
        LOG_ERR("Failed to write to I2C device address %x at reg. %x \n\r", eink_1in9_com.addr,boost_tson[0]);
        return;
    }

	k_msleep(10);

	EPD_1in9_Temperature();
}



void display_values(float temperature, float humidity) {
	updateTemperatureDisplay(temperature);
	updateHumidityDisplay(humidity);

	EPD_1in9_Write_Screen(eink_segments);
}


void display_power_off() {

}

void display_clean() {
    EPD_1in9_lut_5S();
    unsigned char DSPNUM_1in9_off[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       };  // all white
    EPD_1in9_Write_Screen(DSPNUM_1in9_off);
}