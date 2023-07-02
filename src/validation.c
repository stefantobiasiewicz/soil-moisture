#include "validation.h"

#define CAL_START 1
#define CAL_LOW 2
#define CAL_HIGH 4
#define CAL_END 8



int validateCalibration(int value) {
    if (value == CAL_START || value == CAL_LOW || value == CAL_HIGH || value == CAL_END) {
        return ERROR_OK;
    } else {
        return ERROR_INVALID_CALIBRATION;
    }
}

int validateTimeInterval(int value) {
    if (value >= 0 && value <= 65535) {
        return ERROR_OK;
    } else {
        return ERROR_OUT_OF_RANGE_INTERVAL;
    }
}

int validateSoilMoisture(int value) {
    if (value >= 0 && value <= 1000) {
        return ERROR_OK;
    } else {
        return ERROR_OUT_OF_RANGE_SOIL_MOISTURE;
    }
}

int validateBatteryLevel(int value) {
    if (value >= 0 && value <= 1000) {
        return ERROR_OK;
    } else {
        return ERROR_OUT_OF_RANGE_BATTERY_LEVEL;
    }
}

int is_pointer_null(void* pointer) {
    if (pointer == NULL) {
        return ERROR_NULL_POINTER;
    } else {
        return ERROR_OK;
    }
}
