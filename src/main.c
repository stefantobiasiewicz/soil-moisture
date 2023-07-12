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
	.read_adc_mv_moisture = hardware_read_adc_mv_moisture
};


static struct notify_api_t notify = 
{
    .send_notification = ble_send_notify
};

static struct flash_api_t flash = 
{
	.read_calibration_data = flash_read_data,
	.write_calibration_data = flash_write_data,
	.read_notification_time = flash_read_notification_time,
	.write_notification_time = flash_write_notification_time
};

static struct application_api application = {
	.app_calibrate = app_calibrate,
	.app_get_notification_time = app_get_notification_time,
	.app_set_notification_time = app_set_notification_time
};

void error() {
	LOG_ERR("Application occur error rebooting.");
	k_sleep(K_MSEC(3000));
	sys_reboot(0);
}

void main(void)
{	
	LOG_INF("Soil meter application start.");

	if (hardware_init() != ERROR_OK) {
		error();
	}

	if (flash_init() != ERROR_OK) {
		error();
	}

	if (ble_init(&application) != ERROR_OK) {
		error();
	}

	if (app_init(&notify, &hardware, &flash) != ERROR_OK) {
		error();
	}	

	app_main_loop();
}
