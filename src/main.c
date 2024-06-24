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
#include "app.h"
#include "error.h"
#include "flash.h"
#include "ble.h"


LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static struct hardware_api_t hardware = {
    .read_adc_mv_battery = hardware_read_adc_mv_battery,
	.read_adc_mv_moisture = hardware_read_adc_mv_moisture,

	.blue_led_pulse_start = hardware_blue_led_pulse_start,
	.led_off = hardware_led_off,
	.purple_led = hardware_purple_led
};

static struct hardware_callback_t hardware_callbacks = {
	.app_button_press = app_button_press
};


static struct ble_api_t ble = 
{
    .send_notification = ble_send_notify,
	.ble_advertise_connection_start = ble_advertise_connection_start,
	.ble_advertise_connection_stop = ble_advertise_connection_stop,
	.ble_advertise_not_connection_data_start = ble_advertise_not_connection_data_start,
	.ble_advertise_not_connection_data_stop = ble_advertise_not_connection_data_stop
};

static struct flash_api_t flash = 
{
	.read_calibration_data = flash_read_calibration_data,
	.write_calibration_data = flash_write_calibration_data,
	.read_sleep_time = flash_read_sleep_time,
	.write_sleep_time = flash_write_sleep_time
};

static struct application_api application = {
	.app_calibrate = app_calibrate,
	.app_get_sleep_time = app_get_sleep_time,
	.app_set_sleep_time = app_set_sleep_time,
	.app_connected = app_connected,
	.app_disconnected = app_disconnected
};

void error() {
	LOG_ERR("Application occur error rebooting.");
	k_sleep(K_MSEC(3000));
	sys_reboot(0);
}

int main(void)
{	
	/**
	 * for tests
	*/
	// LOG_WRN("TEST APLICATION FOR VOLTAGE TEST");

	// if (hardware_init(&hardware_callbacks) != ERROR_OK) {
	// 	error();
	// }

	// while (1) {
	// 	int battery_mv = hardware_read_adc_mv_battery();
	// 	int soil_mv = hardware_read_adc_mv_moisture();

	// 	LOG_INF("[batery, soil]: [%d,%d]", battery_mv, soil_mv);
	// 	k_sleep(K_MSEC(500));
	// }


	LOG_INF("Soil meter application start.");

	if (hardware_init(&hardware_callbacks) != ERROR_OK) {
		error();
	}

	if (flash_init() != ERROR_OK) {
		error();
	}

	if (ble_init(&application) != ERROR_OK) {
		error();
	}

	if (app_init(&ble, &hardware, &flash) != ERROR_OK) {
		error();
	}	

	app_main_loop();

	return 0;
}
