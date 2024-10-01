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

#include "hardware.h"
#include "flash.h"
#include "ble.h"
#include <stdlib.h>


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);


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


void app_left_button_press();
void app_right_button_press();
static struct hardware_callback_t hardware_callbacks = {
	.app_left_button_press = app_left_button_press,
	.app_right_button_press = app_right_button_press
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


int main(void)
{	
    LOG_INF("###############################################");
    LOG_INF("##                                           ##");
    LOG_INF("##         Soil Meter Application Start      ##");
    LOG_INF("##                                           ##");
    LOG_INF("###############################################");

	if (hardware_init(&hardware_callbacks) != ERROR_OK) {
		error();
	}

	if (flash_init() != ERROR_OK) {
		error();
	}

	// if (ble_init(&application) != ERROR_OK) {
	// 	error();
	// }

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

    while(1) {
        k_msleep(1000);
        k_cpu_idle();
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
    hardware_blue_led_pulse_start();
}

void left_long_click(void) {
    LOG_INF("Left button long click detected.");
    hardware_led_off();
}

void right_click(void) {
    LOG_INF("Right button single click detected.");
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

        k_msleep(10000);
    }
}