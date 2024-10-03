#ifndef DISPLAY_HH
#define DISPLAY_HH


#include <zephyr/kernel.h>
#include "firmware/hardware.h"
#include "validation.h"
#include <zephyr/logging/log.h>

typedef void (*pin_reset_set_f)(bool state);
typedef bool (*pin_busy_read_f)(void);

typedef struct {
    pin_reset_set_f pin_reset_set;
    pin_busy_read_f pin_busy_read;
} eink_1in9_pins_t;


void display_init(eink_1in9_pins_t *eink_1in9_pins);

void display_power_on();
void display_values(float temperature, float humidity);
void display_power_off();


#endif