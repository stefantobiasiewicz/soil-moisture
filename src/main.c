/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

#include "firmware/hardware.h"
#include "firmware/flash.h"
#include "conectivity/ble.h"

#include "display.h"
#include "logic_control.h"

#include <stdlib.h>


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);


soil_moisture_calib_data_t soil_moisture_calib_data = {
    .dry_value = 1495,
    .wet_value = 2702,
};

deivce_config_t device_config = {
    .ble_enable = false,
    .display_enable = true,
};


/**
 * utils
 */
void log_current_thread_info() {
    k_tid_t thread_id = k_current_get();

    LOG_INF("###########################################");
    LOG_INF("Logging current thread information:");
    LOG_INF("Thread ID: %p", thread_id);
    LOG_INF("Thread Name: %s", k_thread_name_get(thread_id));
    LOG_INF("###########################################");
}


void error() {
	LOG_ERR("Application occur error rebooting.");
	k_sleep(K_MSEC(3000));
	sys_reboot(0);
}

measurments_t make_measurements(void);

void app_left_button_press();
void app_right_button_press();
static struct hardware_callback_t hardware_callbacks = {
	.app_left_button_press = app_left_button_press,
	.app_right_button_press = app_right_button_press
};


void pin_reset_set(bool set) {
    if (set) {
        hardware_eink_rst_active();   
    } else {
        hardware_eink_rst_inactive();
    } 
}

bool pin_busy_read() {
    return hardware_eink_busy_read() == 1 ? true : false;
}

eink_1in9_pins_t eink_1in9_pins = {
    .pin_reset_set = pin_reset_set, 
    .pin_busy_read = pin_busy_read
};



#define BUTTONS_STACK_SIZE 1024
#define BUTTONS_THREAD_PRIORITY 2

void buttons_thread(void *, void *, void *);

K_THREAD_STACK_DEFINE(buttons_stack_area, BUTTONS_STACK_SIZE);
struct k_thread buttons_thread_data;
k_tid_t buttons_tid;

#define PERIODIC_STACK_SIZE 1024
#define PERIODIC_THREAD_PRIORITY 2

void periodic_thread(void *, void *, void *);

K_THREAD_STACK_DEFINE(periodic_thread_area, PERIODIC_STACK_SIZE);
struct k_thread periodic_thread_data;
k_tid_t periodic_tid;

k_tid_t main_tid;

typedef struct {
    uint16_t type;
} button_msgq_item_t;
K_MSGQ_DEFINE(button_msgq, sizeof(button_msgq_item_t), 5, 1);


int main(void)
{	
    LOG_INF("###############################################");
    LOG_INF("##                                           ##");
    LOG_INF("##         Soil Meter Application Start      ##");
    LOG_INF("##                                           ##");
    LOG_INF("###############################################");

    hardware_init_status_t init_status = hardware_init(&hardware_callbacks);
	if (init_status.error != ERROR_OK) {
		error();
	}

    device_config.hardware_init_status = init_status;

	if (flash_init() != ERROR_OK) {
		error();
	}

	// if (ble_init(&application) != ERROR_OK) {
	// 	error();
	// }

    if(device_config.display_enable) {
        hardware_power_up();
        hardware_power_internal_up();

        display_init(&eink_1in9_pins);
        display_clean();
    }


    buttons_tid = k_thread_create(&buttons_thread_data, buttons_stack_area,
                                 K_THREAD_STACK_SIZEOF(buttons_stack_area),
                                 buttons_thread,
                                 NULL, NULL, NULL,
                                 BUTTONS_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(buttons_tid, "Buttons_thread");
    
    periodic_tid = k_thread_create(&periodic_thread_data, periodic_thread_area,
                                K_THREAD_STACK_SIZEOF(periodic_thread_area),
                                periodic_thread,
                                NULL, NULL, NULL,
                                BUTTONS_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(periodic_tid, "Periodic_thread");


    main_tid = k_current_get();
    k_thread_name_set(main_tid, "Main_thread");

    while(1) {
        k_msleep(10);
        
        button_msgq_item_t data;
        if (k_msgq_get(&button_msgq, &data, K_NO_WAIT) == 0) {
            if (data.type == 1) {
                if(device_config.ble_enable) {
                    //ble_advertise_connection_start();
                    // led blue pulsing
                    //sleep
                    //connection end
                }
            }
        }

        k_thread_suspend(main_tid);
    }
}

/**
 * Buttons thread handle all button related user input
 */
static volatile int left_button_click_count = 0;
static volatile int right_button_click_count = 0;

void app_left_button_press() {
    LOG_INF("app left button press");

    left_button_click_count++;
    k_thread_resume(buttons_tid);
}
void app_right_button_press() {
    LOG_INF("app right button press");

    right_button_click_count++;
    k_thread_resume(buttons_tid);
}

#define LONG_CLICK_THRESHOLD_MS 1000


void left_click(void) {
    LOG_INF("Left button single click detected.");
    hardware_led_pulse_start(255, 0, 0, K_MSEC(2000));
}

void left_long_click(void) {
    LOG_INF("Left button long click detected.");
    hardware_led_off();
}

void right_click(void) {
    LOG_INF("Right button single click detected.");

    //put to queue
    button_msgq_item_t data = {
        .type = 1,
    };
    while (k_msgq_put(&button_msgq, &data, K_NO_WAIT) != 0) {
        /* message queue is full: purge old data & try again */
        k_msgq_purge(&button_msgq);
    }

    k_thread_resume(main_tid);   
}

void right_long_click(void) {
    LOG_INF("Right button long click detected.");
}

void buttons_thread(void *, void *, void *) {
    log_current_thread_info();

    while (1)
    {
        LOG_INF("hello from button thread loop.");


        // left/right: one click, double click, long click, 
        // both click, both long press

        k_usleep(1000);

        if (check_left_button_pressed()) {
            k_usleep(400);  

            bool long_click = false;
            uint64_t press_start_time = k_uptime_get();  

            while (check_left_button_pressed()) {
                if ((k_uptime_get() - press_start_time) >= LONG_CLICK_THRESHOLD_MS) {
                    long_click = true; 
                    break;
                }
                k_usleep(5000); 
            }

            if (long_click) {
                left_long_click();
            } else {
                left_click();
            }
        }

        if (check_right_button_pressed()) {
            k_usleep(400);  

            bool long_click = false;
            uint64_t press_start_time = k_uptime_get();  

            while (check_right_button_pressed()) {
                if ((k_uptime_get() - press_start_time) >= LONG_CLICK_THRESHOLD_MS) {
                    long_click = true; 
                    break;
                }
                k_usleep(5000); 
            }

            if (long_click) {
                right_long_click();
            } else {
                right_click();
            }
        }

        k_msleep(200);
        k_thread_suspend(buttons_tid);
    }
}

/**
 * Periodic thread handle:
 * soil measurements
 * checking if data is ready to send (difference or sending timeout)
 * displaying data
 * sending data
 * checking battery
 */
void periodic_thread(void *, void *, void *) {
    log_current_thread_info();

    while (1)
    {
        LOG_INF("Periodic measurments and data sharing.");

        /*
            power up
            battery
            temperature 
            soil 

            check if need to update display
                display
            check if need to publish by ble
                publish
        */
        hardware_power_up();

        measurments_t results = make_measurements();

        bool notify = check_parameters_changes(results.soil_moisture, results.ground_temperature, results.battery, k_uptime_get());

        if(notify) {
            if(device_config.display_enable) {
                if (true) {
                    hardware_power_internal_up();

                    display_power_on();
                    display_values(results.ground_temperature, results.soil_moisture);

                    k_msleep(100);
                    
                    hardware_power_internal_down();
                }
            }

            if(device_config.ble_enable) {
                // if(value_schages)
                // ble_advertise_not_connection_data_start(...);
                // k_msleep(10000);
                // ble_advertise_not_connection_data_stop();    
            }
        }

        hardware_power_down();

        k_msleep(get_next_wakeup_time_ms());
    }
}



measurments_t make_measurements(void) {
    measurments_t result = {0};
 
    k_msleep(20);

    result.battery_mv_raw = hardware_read_adc_mv_battery();
    result.battery = vbat_calculate_battery(result.battery_mv_raw);
    LOG_INF("Battery raw (mV): %d, Calculated battery: %.2f", result.battery_mv_raw, result.battery);
    k_msleep(20);

    result.temperature_mv_raw = hardware_read_adc_mv_tempNTC();
    result.ground_temperature = ntc_calcualte_temperatrue(result.temperature_mv_raw);
    LOG_INF("Temperature raw (mV): %d, Calculated temperature: %.2f", result.temperature_mv_raw, result.ground_temperature);
    k_msleep(20);

    hardware_genrator_on();
    k_msleep(100);
    
    result.soil_moisture_mv_raw = hardware_read_adc_mv_moisture();
    hardware_genrator_off();
    LOG_INF("Soil moisture raw (mV): %d", result.soil_moisture_mv_raw);
    
    float soil_moisture = capacitive_sensor_calculate_moisture(result.soil_moisture_mv_raw, soil_moisture_calib_data);
    LOG_INF("Calculated soil moisture: %.2f", result.soil_moisture);
    k_msleep(20);


    if (device_config.hardware_init_status.veml7700_avaliavle) {
        hardware_power_internal_up();
        k_msleep(300);      

        const struct device * veml7700_dev = get_inited_veml7700();
        struct sensor_value lux_value;

        if (sensor_sample_fetch(veml7700_dev) < 0) {
            LOG_ERR("Error feching data from VEML7700");
        }

        if (sensor_channel_get(veml7700_dev, SENSOR_CHAN_LIGHT, &lux_value) < 0) {
            LOG_ERR("Error getting data from VEML7700");
        }

        result.lux = sensor_value_to_float(&lux_value);
        LOG_INF("Light sense: %d.%06d lux", lux_value.val1, lux_value.val2);
    }

    k_msleep(20);
    if (device_config.hardware_init_status.sht40_avaliavle) {
        hardware_power_internal_up();

        const struct device * sht40_dev = get_inited_sht40();
        struct sensor_value temp_value, humidity_value;


        if (sensor_sample_fetch(sht40_dev) < 0) {
            LOG_ERR("Error feching data from SHT40");
        }

        if (sensor_channel_get(sht40_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_value) < 0) {
            LOG_ERR("Error getting data from SHT40");
        }

        if (sensor_channel_get(sht40_dev, SENSOR_CHAN_HUMIDITY, &humidity_value) < 0) {
            LOG_ERR("Error getting data from SHT40");
        }

        result.air_temperature = sensor_value_to_float(&temp_value);
        result.air_humidity = sensor_value_to_float(&humidity_value);

        LOG_INF("Temperature: %d.%06d °C", temp_value.val1, temp_value.val2);
        LOG_INF("Humidity: %d.%06d %%", humidity_value.val1, humidity_value.val2);
    }

    hardware_power_internal_down();
}