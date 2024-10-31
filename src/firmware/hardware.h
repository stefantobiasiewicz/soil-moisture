#ifndef HARDWARE_HH
#define HARDWARE_HH


#include "../main.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>


#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>




extern const struct i2c_dt_spec eink_1in9_com;
extern const struct i2c_dt_spec eink_1in9_data;

typedef void (*app_button_press_t)();


struct hardware_callback_t
{
    app_button_press_t app_left_button_press;
    app_button_press_t app_right_button_press;
};

hardware_init_status_t hardware_init(struct hardware_callback_t *callbacks_p);

bool check_left_button_pressed();
bool check_right_button_pressed();


/**
 * soil generator
 */
void hardware_genrator_on();
void hardware_genrator_off();

/**
 * powering internal 3v3 
 */
void hardware_power_up();
void hardware_power_down();
/**
 * powering 3v3-internal bus use for display and extensions
 */
void hardware_power_internal_up();
void hardware_power_internal_down();


int hardware_read_adc_mv_battery(void);
int hardware_read_adc_mv_moisture(void);
int hardware_read_adc_mv_tempNTC(void);
int hardware_read_adc_mv_vcc(void);

void hardware_set_led_color(uint8_t red, uint8_t green, uint8_t blue);
void hardware_led_pulse_start(uint8_t red, uint8_t green, uint8_t blue, k_timeout_t timeout);
void hardware_led_off(void);




const struct device * get_inited_veml7700();
const struct device * get_inited_sht40();


#endif