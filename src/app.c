#include "app.h"

static struct notify_api notify_api_iml;
static struct hardware_api hardware_api_imp;

static uint16_t map_value_battery(uint16_t battery_adc) {
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


int app_init(struct notify_api * notify, struct hardware_api * hardware) {
    int err = isPointerNull(notify);
    if (err != VALIDATION_OK) {
        // todo logger
        return err;
    }

    err = isPointerNull(hardware);
    if (err != VALIDATION_OK) {
        // todo logger
        return err;
    }

    err = isPointerNull(notify->send_notification);
    if (err != VALIDATION_OK) {
        // todo logger
        return err;
    }
    notify_api_iml.send_notification = notify->send_notification;


    err = isPointerNull(hardware->read_adc_mv_battery);
    if (err != VALIDATION_OK) {
        // todo logger
        return err;
    }
    
    err = isPointerNull(hardware->read_adc_mv_moisture);
    if (err != VALIDATION_OK) {
        // todo logger
        return err;
    }
    
    hardware_api_imp.read_adc_mv_battery = hardware->read_adc_mv_battery;
    hardware_api_imp.read_adc_mv_moisture = hardware->read_adc_mv_moisture;
}


void app_main_loop(void) {

}