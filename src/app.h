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


typedef int (*ble_advertise_connection_start_t)();
typedef int (*ble_advertise_connection_stop_t)();

/**
 * advertising soil and battery data
*/
typedef int (*ble_advertise_not_connection_data_start_t)(uint16_t soil_value, uint16_t battery_value, uint16_t id);
typedef int (*ble_advertise_not_connection_data_stop_t)();

struct ble_api_t
{
    notify_soil_api_t send_notification;

    ble_advertise_connection_start_t ble_advertise_connection_start;
    ble_advertise_connection_stop_t ble_advertise_connection_stop;
    ble_advertise_not_connection_data_start_t ble_advertise_not_connection_data_start;
    ble_advertise_not_connection_data_stop_t ble_advertise_not_connection_data_stop;
};

typedef int (*read_adc_mv_moisture_t)(void);
typedef int (*read_adc_mv_battery_t)(void);
typedef int (*blue_led_pulse_start_t)(void);
typedef int (*purple_led_t)(void);
typedef int (*led_off_t)(void);



struct hardware_api_t
{
    read_adc_mv_moisture_t    read_adc_mv_moisture;
    read_adc_mv_battery_t     read_adc_mv_battery;

    blue_led_pulse_start_t    blue_led_pulse_start;
    purple_led_t purple_led;
    led_off_t     led_off;
};

typedef int (*read_calibration_data_t)(soil_calibration_t *);
typedef void (*write_calibration_data_t)(soil_calibration_t);
typedef int (*read_sleep_time_t)(uint16_t *);
typedef void (*write_sleep_time_t)(uint16_t);

struct flash_api_t
{
    read_calibration_data_t     read_calibration_data;
    write_calibration_data_t    write_calibration_data;
    read_sleep_time_t    read_sleep_time;
    write_sleep_time_t   write_sleep_time;
};

uint8_t app_init(struct ble_api_t * notify, struct hardware_api_t * hardware, struct flash_api_t * flash);
void app_main_loop(void);


/**
 * calibration API
*/
void app_calibrate(uint16_t);

/**
 * notification time API
*/
void app_set_sleep_time(uint16_t seconds);
uint16_t app_get_sleep_time(void);

/**
 * radio callbacks
*/

void app_connected();
void app_disconnected();

/**
 * UX api 
*/
void app_button_press();


#endif