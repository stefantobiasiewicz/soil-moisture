#ifndef NTC_HH
#define NTC_HH
#include "main.h"

float ntc_calcualte_temperatrue(float adc_mv);

float capacitive_sensor_calculate_moisture(uint16_t raw_value, soil_moisture_calib_data_t calibration_data);

float vbat_calculate_battery(uint16_t raw_value);

bool check_parameters_changes(float soil_moisture, float ground_temperature, float battery, uint64_t up_time_ms);
int get_next_wakeup_time_ms();

#endif