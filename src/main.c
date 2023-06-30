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


void main(void)
{
	hardware_init();


	
	while(1) {
		printk("soil moisture = %"PRId32" mV\n", hardware_read_adc_mv_moisture());
		printk("battery = %"PRId32" mV\n", hardware_read_adc_mv_battery());

		k_sleep(K_MSEC(1000));
	}
}
