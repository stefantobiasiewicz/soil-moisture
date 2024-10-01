#ifndef MAIN_HH_APP
#define MAIN_HH_APP

#include <zephyr/kernel.h>

#define ERROR_OK 0

#define ERROR_INVALID_CALIBRATION -1
#define ERROR_OUT_OF_RANGE_INTERVAL -2
#define ERROR_OUT_OF_RANGE_SOIL_MOISTURE -3
#define ERROR_OUT_OF_RANGE_BATTERY_LEVEL -4

#define ERROR_FLASH_INIT -10

#define ERROR_NULL_POINTER -249

typedef struct  {
	uint16_t battery_value;
	uint16_t soil_value;
} measure_data_t;

#endif