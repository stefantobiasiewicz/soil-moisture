/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>

#include "hardware.h"
#include "app.h"

static struct hardware_api hardware_api_imp = {
    .read_adc_mv_battery = hardware_read_adc_mv_battery,
	.read_adc_mv_moisture = hardware_read_adc_mv_moisture
};

static struct notify_api notify_api_imp = 
{
    .send_notification = NULL
};

void error() {
	while (1) {
		;
		// todo implement init error
	}
}

void main(void)
{	
	if (hardware_init() != VALIDATION_OK) {
		error();
	}
	
	if (app_init(&notify_api_imp, &hardware_api_imp) != VALIDATION_OK) {
		error();
	}
	
	while(1) {
		app_main_loop();

		printk("soil moisture = %"PRId32" mV\n", hardware_read_adc_mv_moisture());
		printk("battery = %"PRId32" mV\n", hardware_read_adc_mv_battery());

		k_sleep(K_MSEC(1000));
	}
}
