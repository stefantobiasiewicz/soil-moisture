#ifndef FLASH_HH
#define FLASH_HH

#include <zephyr/kernel.h>

typedef struct {
    uint16_t soil_adc_min;
    uint16_t soil_adc_max;
} soil_calibration_t;

// Inicjalizuje flash
int flash_init();

// Zapisuje wartość 1 pod wskazany adres w flash
int flash_write_data(soil_calibration_t data);

// Odczytuje wartość spod wskazanego adresu w flash
int flash_read_data(soil_calibration_t* data);

#endif
