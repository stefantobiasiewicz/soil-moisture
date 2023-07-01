#ifndef VALIDATION_HH
#define VALIDATION_HH

#include <zephyr/kernel.h>


#define VALIDATION_OK 0

#define ERROR_INVALID_CALIBRATION -1
#define ERROR_OUT_OF_RANGE_INTERVAL -2
#define ERROR_OUT_OF_RANGE_SOIL_MOISTURE -3
#define ERROR_OUT_OF_RANGE_BATTERY_LEVEL -4

#define ERROR_NULL_POINTER -249

int validateCalibration(int value);

int validateTimeInterval(int value);

int validateSoilMoisture(int value);

int validateBatteryLevel(int value);

int isPointerNull(void* pointer);

#endif