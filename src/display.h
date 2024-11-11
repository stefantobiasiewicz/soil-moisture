#ifndef DISPLAY_HH
#define DISPLAY_HH


#include <zephyr/kernel.h>
#include "firmware/hardware.h"
#include "validation.h"
#include <zephyr/logging/log.h>
#include "main.h"

#define CONFIG_EPD_2IN13B_V4

void display_init();
void display_exit_low_power();

void display_values(measurments_t measuremet);
void display_clean();



#endif