#ifndef FLASH_HH
#define FLASH_HH

#include <zephyr/kernel.h>

typedef struct {
    uint16_t soil_adc_soil_min;
    uint16_t soil_adc_soil_max;
    //uint16_t soil_adc_water;
} soil_calibration_t;

// Inicjalizuje flash
int flash_init();

void flash_write_calibration_data(soil_calibration_t data);
int flash_read_calibration_data(soil_calibration_t* data);

void flash_write_sleep_time(uint16_t data);
int flash_read_sleep_time(uint16_t* data);

#endif
