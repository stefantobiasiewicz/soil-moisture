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

typedef struct  {
	uint16_t dry_value;
	uint16_t wet_value;
} soil_moisture_calib_data_t;

typedef struct {
    bool error;
    bool veml7700_avaliavle;
    bool sht40_avaliavle;
} hardware_init_status_t;

typedef struct {
	bool ble_enable;
	bool display_enable;
    bool periodic_thread_suspend;
    bool periodic_thread_fast;
    bool zibee_enable;

	hardware_init_status_t hardware_init_status;
} deivce_config_t;

typedef struct {
    int battery_mv_raw;
    float battery_percent;
    int temperature_ground_mv_raw;
    float temperature_ground;
    int soil_moisture_mv_raw;
    float soil_moisture_percent;
    float lux;
    float air_temperature;
    float air_humidity;
} measurments_t;


/**
 * data is multyply by 10 so 235 == 23,5%
 */
typedef struct {
    uint16_t battery;
    uint16_t ground_temperature;
    uint16_t soil_moisture_percent;
    uint16_t lux;
    uint16_t air_temperature;
    uint16_t air_humidity;
} measurments_ble_adv_data_t;


#endif