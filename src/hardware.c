#include "hardware.h"

#include <zephyr/drivers/pwm.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hardware, LOG_LEVEL_DBG);


#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

static const struct adc_dt_spec adc_soil_chan =
    ADC_DT_SPEC_GET_BY_IDX(ZEPHYR_USER_NODE, 0);
static const struct adc_dt_spec adc_vcc_chan =
    ADC_DT_SPEC_GET_BY_IDX(ZEPHYR_USER_NODE, 1);

const struct gpio_dt_spec sensor_power = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, signal_gpios);

const struct pwm_dt_spec generator = PWM_DT_SPEC_GET_BY_NAME(ZEPHYR_USER_NODE, generator);


static void enable_sensor() {
    gpio_pin_set_dt(&sensor_power, 1);

    pwm_set_dt(&generator, 5000 , 2500); 
}

static void disable_sensor() {
    gpio_pin_set_dt(&sensor_power, 0);

    pwm_set_dt(&generator, 5000 , 0); 
}


int hardware_init(void) {
    int err = 0;

	if (!device_is_ready(generator.dev)) {
		printk("Pwm for soil sensor device not ready\n");
		return -1;
	}

	if (!device_is_ready(sensor_power.port)) {
		printk("Gpio for powering soil sensor device not ready\n");
		return -1;
	}

	err = gpio_pin_configure_dt(&sensor_power, GPIO_OUTPUT);
	if (err < 0) {
		printk("Could not setup gpio for powering soil sensor (%d)\n", err);
		return -1;
	}

    disable_sensor();

	if (!device_is_ready(adc_soil_chan.dev)) {
		printk("ADC controller for soil device not ready\n");
		return -1;
	}

	err = adc_channel_setup_dt(&adc_soil_chan);
	if (err < 0) {
		printk("Could not setup adc soil channel (%d)\n", err);
		return -1;
	}

	if (!device_is_ready(adc_vcc_chan.dev)) {
		printk("ADC controller for vcc device not ready\n");
		return -1;
	}

	err = adc_channel_setup_dt(&adc_vcc_chan);
	if (err < 0) {
		printk("Could not setup adc vcc channel (%d)\n", err);
		return -1;
	}

    return err;
}



static int read_mv_from_adc(struct adc_dt_spec * adc) {
    LOG_INF("hardware: reading adc form chanel with id: [%d].", adc->channel_id);

    int err = 0;

    int16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		.buffer_size = sizeof(buf),
	};

    int32_t val_mv;

    (void)adc_sequence_init_dt(adc, &sequence);

    err = adc_read(adc->dev, &sequence);
    if (err < 0) {
//todo  handle error
        printk("Could not read (%d)\n", err);
        return -1;
    } 

    /* conversion to mV may not be supported, skip if not */
    val_mv = buf;
    err = adc_raw_to_millivolts_dt(adc,
                        &val_mv);
    if (err < 0) {
//todo  handle error
        printk(" (value in mV not available)\n");
        return -1;
    }

    LOG_INF("hardware: result: [%d].", val_mv);
    return val_mv;
}

int hardware_read_adc_mv_moisture(void) {
    enable_sensor();
    k_sleep(K_MSEC(30));
    
    int result = read_mv_from_adc(&adc_soil_chan);

    disable_sensor();

    return result;
}

int hardware_read_adc_mv_battery(void) {
    return read_mv_from_adc(&adc_vcc_chan);
}




