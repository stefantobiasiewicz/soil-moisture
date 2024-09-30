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




void app_button_press();
static struct hardware_callback_t hardware_callbacks = {
	.app_button_press = app_button_press
};



// #define ISR_THREAD_STACK_SIZE 2048
// #define MY_PRIORITY 5

// void isr_thread(void *, void *, void *);

// K_THREAD_DEFINE(my_tid, ISR_THREAD_STACK_SIZE,
//                 isr_thread, NULL, NULL, NULL,
//                 MY_PRIORITY, 0, 0);


int main(void)
{	
	LOG_INF("Soil meter application start.");

	if (hardware_init(&hardware_callbacks) != ERROR_OK) {
		error();
	}

	if (flash_init() != ERROR_OK) {
		error();
	}

	// if (ble_init(&application) != ERROR_OK) {
	// 	error();
	// }
    

    while(1) {
        k_cpu_idle();
    }
}

/**
 * ISR corelated functions
*/

K_SEM_DEFINE(isr_sem, 0, 1);
// K_MUTEX_DEFINE(isr_sem);


volatile int button_press_count = 0;
void app_button_press() {
    LOG_INF("app button press");
    k_sem_give(&isr_sem);
	button_press_count ++;
}

int button_code = 0;

void isr_thread(void *, void *, void *) {
    while (1)
    {
		// button click detection
		if(k_sem_take(&isr_sem, K_FOREVER) == 0) {
			k_sleep(K_MSEC(300));
			if(button_press_count > 1) {
				// kliknęty kilka razy
				button_code = 3;
			}
            else {
                k_sleep(K_MSEC(20));
                if (check_button_pressed()) { // juz nie trzymany
                    button_code = 2;	// presss
                }
                k_sleep(K_MSEC(20));
                if (!check_button_pressed()) { // juz nie trzymany
                    button_code = 1;	// jedno przytrzymanie
                }
            }
			//use it
			LOG_INF("button press code: %d", button_code);
            button_press_count = 0;
        }
		k_sleep(K_MSEC(30));


        // if(k_sem_take(&isr_sem, K_FOREVER) == 0) {
        //     if(application_state == BUTTON_ACTION) {
            
        //         ble_advertise_connection_start();
        //         blue_led_pulse_start();

        //         k_sleep(K_SECONDS(10));
        //         ble_advertise_connection_stop();

        //         if(application_state != CONECTED_TO_USER) {
        //             application_state = NORMAL_WORK;
        //             led_off();
        //         }
        //     }
        // }
        // k_sleep(K_MSEC(500));
    }
}


/*


main thread - inicjuje hardware 

timer thread - thread started by timer and peridoic measurements

buttion thread


*/