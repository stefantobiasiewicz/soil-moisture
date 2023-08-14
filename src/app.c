#include "app.h"
#include "error.h"

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#define CAL_START 1
#define CAL_WET 2
#define CAL_DRY 4
#define CAL_END 8


#define ALPHA 0.2 

struct dsp_lpf_t {
    int prev_output;
};

enum state {WORK = 0, CONNECTION, BUTTON_ACTION};

/**
 * application logic memory start
*/
static enum state application_state = WORK;

static soil_calibration_t soil_calibration;
static uint16_t sleep_time = 1;
static const uint8_t quick_sleep_time = 3;

static uint16_t unique_id = 0;

static uint16_t battery_value = 0;
static struct dsp_lpf_t battery_filter;

static uint16_t soil_value = 0;
static struct dsp_lpf_t soil_filter;


/**
 * application memory end
*/

/**
 * modules start
*/

static struct ble_api_t ble;
static struct hardware_api_t hardware;
static struct flash_api_t flash;

/**
 * modules end
*/


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

void init_low_pass_filter(struct dsp_lpf_t *filter, int init_value) {
    filter->prev_output = init_value;
}

int update_low_pass_filter(struct dsp_lpf_t *filter, int input) {
    LOG_INF("dsp calcualting data input [%d].", input);
    
    int delta = abs(filter->prev_output - input);

    LOG_INF("dsp filter delta [%d].", delta);

    float alpha = ALPHA;

    if(delta > 500) {
        alpha = 0.9f;
    }

    LOG_INF("dsp filter alpha [%f].", alpha);
    int output = alpha * input + (1 - alpha) * filter->prev_output;
    filter->prev_output = output;
    return output;
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
    soil_calibration.soil_adc_max = hardware.read_adc_mv_moisture() + 10;
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


void app_set_sleep_time(uint16_t seconds) {
    LOG_INF("setting sleep time: [%d] seconds.", seconds);

    sleep_time = seconds;
    flash.write_sleep_time(sleep_time);
}

uint16_t app_get_sleep_time(void) {
    LOG_INF("getting sleep time: [%d] seconds.", sleep_time);
    return sleep_time;
}



static void make_measurments() {
    LOG_INF("making soil measurments.");

    int battery_adc = hardware.read_adc_mv_battery();
    k_sleep(K_USEC(200));

    int moiusture_adc = hardware.read_adc_mv_moisture();

    soil_value = update_low_pass_filter(&soil_filter, map_soil(moiusture_adc));
    battery_value = update_low_pass_filter(&battery_filter, map_battery(battery_adc));
}



/**
 * module main loop
*/
void app_main_loop(void) {
    while(1) {
        switch (application_state)
        {
        case WORK:
            make_measurments();
      
            ble.ble_advertise_not_connection_data_start(soil_value, battery_value, unique_id++);
            k_sleep(K_SECONDS(5));
            ble.ble_advertise_not_connection_data_stop();

            LOG_INF("idle stat go to sleep for: [%ds]", sleep_time);
            k_sleep(K_SECONDS(sleep_time));
            break;
        case CONNECTION:
            make_measurments();

            ble.send_notification(soil_value, battery_value);

            LOG_INF("idle stat go to quick sleep for: [%ds]", quick_sleep_time);
            k_sleep(K_SECONDS(quick_sleep_time));
            break;   
        default:
            k_sleep(K_SECONDS(1));
            break;
        }
    }
}

void app_connected() {
    LOG_INF("app conected via ble status: CONNECTION");
    
    application_state = CONNECTION;

    hardware.purple_led();
}

void app_disconnected() {
    LOG_INF("app disconected via ble status: WORK");

    application_state = WORK;

    hardware.led_off();
}



/**
 * ISR corelated functions
*/
K_SEM_DEFINE(isr_sem, 0, 1);

void app_button_press() {
    LOG_INF("app button press: BUTTON_ACTION");
    application_state = BUTTON_ACTION;
    k_sem_give(&isr_sem);
}

void isr_thread(void *, void *, void *) {
    while (1)
    {
        if(k_sem_take(&isr_sem, K_FOREVER) == 0) {
            if(application_state == BUTTON_ACTION) {
            
                ble.ble_advertise_connection_start();
                hardware.blue_led_pulse_start();

                k_sleep(K_SECONDS(10));
                ble.ble_advertise_connection_stop();

                if(application_state != CONNECTION) {
                    application_state = WORK;
                    hardware.led_off();
                }
            }
        }
        k_sleep(K_MSEC(500));
    }
}

/**
 * initalizing module
*/

#define ISR_THREAD_STACK_SIZE 2048
#define MY_PRIORITY 5

K_THREAD_DEFINE(my_tid, ISR_THREAD_STACK_SIZE,
                isr_thread, NULL, NULL, NULL,
                MY_PRIORITY, 0, 0);

uint8_t app_init(struct ble_api_t * ble_p, struct hardware_api_t * hardware_p, struct flash_api_t * flash_p) {
    LOG_INF("app initializing.");
   
    int err = is_pointer_null(ble_p);
    if (err != ERROR_OK) {
        LOG_ERR("ble_p is NULL.");
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

    err = is_pointer_null(ble_p->send_notification);
    if (err != ERROR_OK) {
        LOG_ERR("ble_p->send_notification is NULL.");
        return err;
    }
    err = is_pointer_null(ble_p->ble_advertise_connection_start);
    if (err != ERROR_OK) {
        LOG_ERR("ble_p->ble_advertise_connection_start is NULL.");
        return err;
    }
    err = is_pointer_null(ble_p->ble_advertise_connection_stop);
    if (err != ERROR_OK) {
        LOG_ERR("ble_p->ble_advertise_connection_stop is NULL.");
        return err;
    }
    err = is_pointer_null(ble_p->ble_advertise_not_connection_data_start);
    if (err != ERROR_OK) {
        LOG_ERR("ble_p->ble_advertise_not_connection_data_start is NULL.");
        return err;
    }
    err = is_pointer_null(ble_p->ble_advertise_not_connection_data_stop);
    if (err != ERROR_OK) {
        LOG_ERR("ble_p->ble_advertise_not_connection_data_stop is NULL.");
        return err;
    }
    ble.send_notification = ble_p->send_notification;
    ble.ble_advertise_connection_start = ble_p->ble_advertise_connection_start;
    ble.ble_advertise_connection_stop = ble_p->ble_advertise_connection_stop;
    ble.ble_advertise_not_connection_data_start = ble_p->ble_advertise_not_connection_data_start;
    ble.ble_advertise_not_connection_data_stop = ble_p->ble_advertise_not_connection_data_stop;


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
    
    err = is_pointer_null(hardware_p->blue_led_pulse_start);
    if (err != ERROR_OK) {
        LOG_ERR("hardware_p->blue_led_pulse_start is NULL.");
        return err;
    }
    
    err = is_pointer_null(hardware_p->purple_led);
    if (err != ERROR_OK) {
        LOG_ERR("hardware_p->purple_led is NULL.");
        return err;
    }
    
    err = is_pointer_null(hardware_p->led_off);
    if (err != ERROR_OK) {
        LOG_ERR("hardware_p->led_off is NULL.");
        return err;
    }
    
    hardware.read_adc_mv_battery = hardware_p->read_adc_mv_battery;
    hardware.read_adc_mv_moisture = hardware_p->read_adc_mv_moisture;
    hardware.blue_led_pulse_start = hardware_p->blue_led_pulse_start;
    hardware.purple_led = hardware_p->purple_led;
    hardware.led_off = hardware_p->led_off;


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

    err = is_pointer_null(flash_p->read_sleep_time);
    if (err != ERROR_OK) {
        LOG_ERR("flash_p->read_sleep_time is NULL.");
        return err;
    }
    
    err = is_pointer_null(flash_p->write_sleep_time);
    if (err != ERROR_OK) {
        LOG_ERR("flash_p->write_sleep_time is NULL.");
        return err;
    }
        
    flash.read_calibration_data = flash_p->read_calibration_data;
    flash.write_calibration_data = flash_p->write_calibration_data;
    flash.read_sleep_time = flash_p->read_sleep_time;
    flash.write_sleep_time = flash_p->write_sleep_time;


    /**
     * set up application data
    */
    flash.read_calibration_data(&soil_calibration);
    flash.read_sleep_time(&sleep_time);

    LOG_WRN("Warm up dsp buffer START...");
    make_measurments();

    init_low_pass_filter(&soil_filter, soil_value);
    init_low_pass_filter(&battery_filter, battery_value);
    LOG_WRN("Warm up dsp buffer DONE...");

    return ERROR_OK;
}
