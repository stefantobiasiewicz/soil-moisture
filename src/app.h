#ifndef APP_HH
#define APP_HH

#include <zephyr/kernel.h>

#include "validation.h"
#include "flash.h"

#define INPUT_MIN_BATTERY 2000
#define INPUT_MAX_BATTERY 3000
#define OUTPUT_MIN_BATTERY 0
#define OUTPUT_MAX_BATTERY 1000

typedef void (*notify_api_t)(uint16_t soil_moisture);

struct notify_api_t
{
    notify_api_t send_notification;
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

int app_init(struct notify_api_t * notify, struct hardware_api_t * hardware, struct flash_api_t * flash);
void app_main_loop(void);


/**
 * calibration API
*/
void app_calibrate_start(void);
void app_calibrate_min(void);
void app_calibrate_max(void);
void app_calibrate_end(void);

/**
 * notification time API
*/
void app_set_notification_time(uint16_t seconds);
uint16_t app_get_notification_time(void);


#endif