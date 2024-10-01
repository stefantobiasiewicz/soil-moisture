#ifndef HARDWARE_HH
#define HARDWARE_HH

#include <zephyr/kernel.h>


 typedef void (*app_button_press_t)();


struct hardware_callback_t
{
    app_button_press_t app_left_button_press;
    app_button_press_t app_right_button_press;
};

int hardware_init(struct hardware_callback_t * callbacks_p);

bool check_left_button_pressed();
bool check_right_button_pressed();

int hardware_read_adc_mv_moisture(void);
int hardware_read_adc_mv_battery(void);

void hardware_set_led_color(uint8_t red, uint8_t green, uint8_t blue);
void hardware_led_pulse_start(uint8_t red, uint8_t green, uint8_t blue, k_timeout_t timeout);
void hardware_led_off(void);

#endif