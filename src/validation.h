#ifndef VALIDATION_HH
#define VALIDATION_HH

#include "error.h"
#include <zephyr/kernel.h>


int validateCalibration(int value);

int validateTimeInterval(int value);

int validateSoilMoisture(int value);

int validateBatteryLevel(int value);

int is_pointer_null(void* pointer);

#endif