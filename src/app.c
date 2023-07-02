#include "app.h"
#include "error.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_DBG);


static struct notify_api_t notify;
static struct hardware_api_t hardware;
static struct flash_api_t flash;

static soil_calibration_t soil_calibration;
static uint16_t notification_time = 1;

static uint16_t map_value_battery(uint16_t battery_adc) {
    LOG_INF("mapping battery value [%d].", battery_adc);

    if (battery_adc <= INPUT_MIN_BATTERY) {
        return OUTPUT_MAX_BATTERY;
    } else if (battery_adc >= INPUT_MAX_BATTERY) {
        return OUTPUT_MIN_BATTERY;
    } else {
        uint16_t outputRangeBattery = OUTPUT_MAX_BATTERY - OUTPUT_MIN_BATTERY;
        uint16_t inputRangeBattery = INPUT_MAX_BATTERY - INPUT_MIN_BATTERY;
        uint16_t mappedValueBattery = ((battery_adc - INPUT_MIN_BATTERY) * outputRangeBattery) / inputRangeBattery + OUTPUT_MIN_BATTERY;
        return mappedValueBattery;
    }
}


int app_init(struct notify_api_t * notify_p, struct hardware_api_t * hardware_p, struct flash_api_t * flash_p) {
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
}


void app_calibrate_start()
{
    LOG_INF("Starting calibration...");
}

void app_calibrate_min()
{
    LOG_INF("Calibrating minimum value...");
    soil_calibration.soil_adc_min = hardware.read_adc_mv_moisture();
}

void app_calibrate_max()
{
    LOG_INF("Calibrating maximum value...");
    soil_calibration.soil_adc_max = hardware.read_adc_mv_moisture();
}

void app_calibrate_end()
{
    LOG_INF("Calibration completed.");
    flash.write_calibration_data(soil_calibration);
}


void app_set_notification_time(uint16_t seconds) {
    LOG_INF("setting notification time: [%d] seconds.", seconds);

    notification_time = seconds;
    flash.write_notification_time(notification_time);
}

uint16_t app_get_notification_time(void) {
    LOG_INF("getting notification time: [%d] seconds.", notification_time);
    return notification_time;
}

void app_main_loop(void) {

}