#include "app.h"
#include "error.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#define CAL_START 1
#define CAL_WET 2
#define CAL_DRY 4
#define CAL_END 8

static struct notify_api_t notify;
static struct hardware_api_t hardware;
static struct flash_api_t flash;

static soil_calibration_t soil_calibration;
static uint16_t notification_time = 1;

// struct k_timer app_timer;

static uint16_t map_battery(int battery_adc) {
    LOG_INF("mapping battery value [%d].", battery_adc);

    if (battery_adc <= INPUT_MIN_BATTERY) {
        return OUTPUT_MIN_BATTERY;
    } else if (battery_adc >= INPUT_MAX_BATTERY) {
        return OUTPUT_MAX_BATTERY;
    } else {
        uint16_t outputRangeBattery = OUTPUT_MAX_BATTERY - OUTPUT_MIN_BATTERY;
        uint16_t inputRangeBattery = INPUT_MAX_BATTERY - INPUT_MIN_BATTERY;
        uint16_t mappedValueBattery = ((battery_adc - INPUT_MIN_BATTERY) * outputRangeBattery) / inputRangeBattery + OUTPUT_MIN_BATTERY;
        return mappedValueBattery;
    }
}


static uint16_t map_soil(int input)
{
    LOG_INF("mapping soil value [%d].", input);
    // Ensure that the input value is within the calibrated range
    if (input < soil_calibration.soil_adc_min) {
        input = soil_calibration.soil_adc_min;
    } else if (input > soil_calibration.soil_adc_max) {
        input = soil_calibration.soil_adc_max;
    }

    // Calculate the mapped value based on the calibrated range
    uint16_t range = soil_calibration.soil_adc_max - soil_calibration.soil_adc_min;
    uint16_t mapped_value = 1000 - ((input - soil_calibration.soil_adc_min) * 1000) / range;

    return mapped_value;
}

/**
 * calibration API
*/
void app_calibrate_start()
{
    LOG_INF("Starting calibration...");

    /**
     * maybe need to tourn on sensor na boost converter for calibartion 
    */

}

static void app_calibrate_wet()
{
    LOG_INF("Calibrating minimum value...");
    soil_calibration.soil_adc_min = hardware.read_adc_mv_moisture();
}

static void app_calibrate_dry()
{
    LOG_INF("Calibrating maximum value...");
    soil_calibration.soil_adc_max = hardware.read_adc_mv_moisture();
}

static void app_calibrate_end()
{
    LOG_INF("Calibration completed.");
    flash.write_calibration_data(soil_calibration);
}


void app_calibrate(uint16_t command) {
    LOG_INF("calibration update.");

    switch (command)
    {
    case CAL_START:
        app_calibrate_start();
        break;
    case CAL_WET:
        app_calibrate_wet();
        break;
    case CAL_DRY:
        /* code */
        app_calibrate_dry();
        break;
    case CAL_END:
        app_calibrate_end();
        break;
    
    default:
        LOG_ERR("unknown calibration command.");
        break;
    }
}


void app_set_notification_time(uint16_t seconds) {
    LOG_INF("setting notification time: [%d] seconds.", seconds);

    notification_time = seconds;
    flash.write_notification_time(notification_time);
    // k_timer_start(&app_timer, K_SECONDS(notification_time), K_SECONDS(notification_time));

    #ifdef CONFIG_THREAD_ANALYZER
       thread_analyzer_print();
    #endif
}

/**
 * setting notification API
*/
uint16_t app_get_notification_time(void) {
    LOG_INF("getting notification time: [%d] seconds.", notification_time);
    return notification_time;
}

static void make_measurments() {
    LOG_INF("making soil measurments.");

    int battery_adc = hardware.read_adc_mv_battery();

    int moiusture_adc = hardware.read_adc_mv_moisture();

    notify.send_notification(map_soil(moiusture_adc), map_battery(battery_adc));
}



/**
 * module main loop
*/
void app_main_loop(void) {
    while(1) {
        make_measurments();
        k_sleep(K_SECONDS(notification_time));
    }
}


/**
 * initalizing module
*/

uint8_t app_init(struct notify_api_t * notify_p, struct hardware_api_t * hardware_p, struct flash_api_t * flash_p) {
    LOG_INF("app initializing.");
   
    int err = is_pointer_null(notify_p);
    if (err != ERROR_OK) {
        LOG_ERR("notify_p is NULL.");
        return err;
    }

    err = is_pointer_null(hardware_p);
    if (err != ERROR_OK) {
        LOG_ERR("hardware_p is NULL.");
        return err;
    }

    err = is_pointer_null(flash_p);
    if (err != ERROR_OK) {
        LOG_ERR("flash_p is NULL.");
        return err;
    }

    err = is_pointer_null(notify_p->send_notification);
    if (err != ERROR_OK) {
        LOG_ERR("notify_p->send_notification is NULL.");
        return err;
    }
    notify.send_notification = notify_p->send_notification;


    err = is_pointer_null(hardware_p->read_adc_mv_battery);
    if (err != ERROR_OK) {
        LOG_ERR("hardware_p->read_adc_mv_battery is NULL.");
        return err;
    }
    
    err = is_pointer_null(hardware_p->read_adc_mv_moisture);
    if (err != ERROR_OK) {
        LOG_ERR("hardware_p->read_adc_mv_moisture is NULL.");
        return err;
    }
    
    hardware.read_adc_mv_battery = hardware_p->read_adc_mv_battery;
    hardware.read_adc_mv_moisture = hardware_p->read_adc_mv_moisture;


    err = is_pointer_null(flash_p->read_calibration_data);
    if (err != ERROR_OK) {
        LOG_ERR("flash_p->read_calibration_data is NULL.");
        return err;
    }
    
    err = is_pointer_null(flash_p->write_calibration_data);
    if (err != ERROR_OK) {
        LOG_ERR("flash_p->write_calibration_data is NULL.");
        return err;
    }

    err = is_pointer_null(flash_p->read_notification_time);
    if (err != ERROR_OK) {
        LOG_ERR("flash_p->read_notification_time is NULL.");
        return err;
    }
    
    err = is_pointer_null(flash_p->write_notification_time);
    if (err != ERROR_OK) {
        LOG_ERR("flash_p->write_notification_time is NULL.");
        return err;
    }
        
    flash.read_calibration_data = flash_p->read_calibration_data;
    flash.write_calibration_data = flash_p->write_calibration_data;
    flash.read_notification_time = flash_p->read_notification_time;
    flash.write_notification_time = flash_p->write_notification_time;


    /**
     * set up application data
    */
    flash.read_calibration_data(&soil_calibration);
    flash.read_notification_time(&notification_time);

    // k_timer_init(&app_timer, make_measurments, NULL);
    // k_timer_start(&app_timer, K_SECONDS(notification_time), K_SECONDS(notification_time));

    return ERROR_OK;
}
