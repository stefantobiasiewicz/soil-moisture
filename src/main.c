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



LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static struct hardware_api_t hardware = {
    .read_adc_mv_battery = hardware_read_adc_mv_battery,
	.read_adc_mv_moisture = hardware_read_adc_mv_moisture
};

void notify_paceholder(uint16_t a) {
	LOG_INF("nitificiation placeholder with value: [%d].", (int) a);
}

static struct notify_api_t notify = 
{
    .send_notification = notify_paceholder
};

static struct flash_api_t flash = 
{
	.read_calibration_data = flash_read_data,
	.write_calibration_data = flash_write_data,
	.read_notification_time = flash_read_notification_time,
	.write_notification_time = flash_write_notification_time
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

	if (app_init(&notify, &hardware, &flash) != ERROR_OK) {
		error();
	}	

	
	while(1) {
		app_main_loop();

		// printk("soil moisture = %"PRId32" mV\n", hardware_read_adc_mv_moisture());
		// printk("battery = %"PRId32" mV\n", hardware_read_adc_mv_battery());

		k_sleep(K_MSEC(1000));
	}
}
