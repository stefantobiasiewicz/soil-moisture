#ifndef APP_HH
#define APP_HH

#include <zephyr/kernel.h>

#include "validation.h"
#include "flash.h"

#define INPUT_MIN_BATTERY 2000
#define INPUT_MAX_BATTERY 3000
#define OUTPUT_MIN_BATTERY 0
#define OUTPUT_MAX_BATTERY 1000

typedef int (*notify_soil_api_t)(uint16_t soil_moisture, uint16_t battery);


struct notify_api_t
{
    notify_soil_api_t send_notification;
};

typedef int (*read_adc_mv_moisture_t)(void);
typedef int (*read_adc_mv_battery_t)(void);

struct hardware_api_t
{
    read_adc_mv_moisture_t    read_adc_mv_moisture;
    read_adc_mv_battery_t     read_adc_mv_battery;
};

typedef int (*read_calibration_data_t)(soil_calibration_t *);
typedef int (*write_calibration_data_t)(soil_calibration_t);
typedef int (*read_notification_time_t)(uint16_t *);
typedef int (*write_notification_time_t)(uint16_t);

struct flash_api_t
{
    read_calibration_data_t     read_calibration_data;
    write_calibration_data_t    write_calibration_data;
    read_notification_time_t    read_notification_time;
    write_notification_time_t   write_notification_time;
};

uint8_t app_init(struct notify_api_t * notify, struct hardware_api_t * hardware, struct flash_api_t * flash);
void app_main_loop(void);


/**
 * calibration API
*/
void app_calibrate(uint16_t);

/**
 * notification time API
*/
void app_set_notification_time(uint16_t seconds);
uint16_t app_get_notification_time(void);


/**
 * todo
 * 
 * After measure inform radio module for:
 *  - ble: start advertising measured data imidietli for 10s/20s/30s ???
 * 
 * 
 * Measurmets min seconds 30s
*/

#endif