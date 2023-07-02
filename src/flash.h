#ifndef FLASH_HH
#define FLASH_HH

#include <zephyr/kernel.h>

typedef struct {
    uint16_t soil_adc_min;
    uint16_t soil_adc_max;
} soil_calibration_t;

// Inicjalizuje flash
int flash_init();

int flash_write_data(soil_calibration_t data);
int flash_read_data(soil_calibration_t* data);

int flash_write_notification_time(uint16_t data);
int flash_read_notification_time(uint16_t* data);

#endif
