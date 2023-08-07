#ifndef HARDWARE_HH
#define HARDWARE_HH

#include <zephyr/kernel.h>

int hardware_init(void);

int hardware_read_adc_mv_moisture(void);
int hardware_read_adc_mv_battery(void);

void set_led_color(uint8_t red, uint8_t green, uint8_t blue);


#endif