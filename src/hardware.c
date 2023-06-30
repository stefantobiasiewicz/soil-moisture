#include "hardware.h"


#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>

static const struct adc_dt_spec adc_soil_chan =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);
static const struct adc_dt_spec adc_vcc_chan =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 1);

static const struct adc_dt_spec adc_channels[] = {
	adc_soil_chan, adc_vcc_chan
};


int hardware_init(void) {
    int err = 0;

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

    return val_mv;
}

int hardware_read_adc_mv_moisture(void) {
    return read_mv_from_adc(&adc_soil_chan);
}

int hardware_read_adc_mv_battery(void) {
    return read_mv_from_adc(&adc_vcc_chan);
}



