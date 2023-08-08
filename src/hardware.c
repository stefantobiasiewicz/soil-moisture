#include "hardware.h"

#include <zephyr/drivers/pwm.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>

#include "validation.h"

LOG_MODULE_REGISTER(hardware, LOG_LEVEL_DBG);

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

static const struct adc_dt_spec adc_soil_chan = ADC_DT_SPEC_GET_BY_IDX(ZEPHYR_USER_NODE, 0);
static const struct adc_dt_spec adc_vcc_chan = ADC_DT_SPEC_GET_BY_IDX(ZEPHYR_USER_NODE, 1);

const struct gpio_dt_spec sensor_power = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, signal_gpios);

const struct pwm_dt_spec generator = PWM_DT_SPEC_GET_BY_NAME(ZEPHYR_USER_NODE, generator);


/**
 * UX
*/
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(DT_ALIAS(button), gpios, {0});
static struct gpio_callback button_cb_data;

static const struct pwm_dt_spec pwm_led_r = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led_r));
static const struct pwm_dt_spec pwm_led_g = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led_g));
static const struct pwm_dt_spec pwm_led_b = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led_b));

static struct hardware_callback_t callbacks;


void set_led_color(uint8_t red, uint8_t green, uint8_t blue);

static void enable_sensor() {
    gpio_pin_set_dt(&sensor_power, 1);

    pwm_set_dt(&generator, 5000 , 2500); 
}

static void disable_sensor() {
    gpio_pin_set_dt(&sensor_power, 0);

    pwm_set_dt(&generator, 5000 , 0); 
}

static struct k_timer pulse_timer;
static int blue_intensity = 0;
static int direction = 1; // Kierunek zmiany intensywności koloru (1 - wzrost, -1 - spadek)
static uint32_t step_msec = 1;
void pulse_blue_color_step() {
    set_led_color(0, 0, blue_intensity);

    blue_intensity += direction;

    // Zmiana kierunku, gdy osiągniemy maksymalny lub minimalny poziom
    if (blue_intensity <= 0 || blue_intensity >= 255) {
        direction = -direction;
    }
}

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());

    callbacks.app_button_press();
}


void set_led_color(uint8_t red, uint8_t green, uint8_t blue) {
    pwm_set_pulse_dt(&pwm_led_r, PWM_USEC(red));
    pwm_set_pulse_dt(&pwm_led_g, PWM_USEC(green));
    pwm_set_pulse_dt(&pwm_led_b, PWM_USEC(blue));
}


int hardware_led_off(void) {
    k_timer_stop(&pulse_timer);
    set_led_color(0,0,0);
}

int hardware_blue_led_pulse_start(void) {
    k_timer_start(&pulse_timer, K_MSEC(0), K_MSEC(step_msec));
}

int hardware_purple_led(void) {
    hardware_led_off();

    set_led_color(255,0,255);
}


static int read_mv_from_adc(const struct adc_dt_spec * adc) {
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



int hardware_init(struct hardware_callback_t * callbacks_p) {
    int err = is_pointer_null(callbacks_p);
    if (err != ERROR_OK) {
        LOG_ERR("callbacks is NULL.");
        return err;
    }

    err = is_pointer_null(callbacks_p->app_button_press);
    if (err != ERROR_OK) {
        LOG_ERR("callbacks_p->app_button_press is NULL.");
        return err;
    }
    callbacks.app_button_press = callbacks_p->app_button_press;


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
    
    /**
     * 
     * adc channels config
     * 
    */

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

    /**
     * 
     * pwm led config
     * 
    */
   	printk("initializing rgb led module\n");
   	if (!device_is_ready(pwm_led_r.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       pwm_led_r.dev->name);
		return -1;
	}


   	if (!device_is_ready(pwm_led_g.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       pwm_led_g.dev->name);
		return -1;
	}


   	if (!device_is_ready(pwm_led_b.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       pwm_led_b.dev->name);
		return -1;
	}

    pwm_set_pulse_dt(&pwm_led_r, PWM_USEC(256));
    pwm_set_pulse_dt(&pwm_led_g, PWM_USEC(256));
    pwm_set_pulse_dt(&pwm_led_b, PWM_USEC(256));


    /**
     * 
     * Button init
     * 
    */
   if (!device_is_ready(button.port)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return -1;
	}

	err = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (err != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       err, button.port->name, button.pin);
		return -1;
	}

	err = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (err != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			err, button.port->name, button.pin);
		return -1;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	printk("Set up button at %s pin %d\n", button.port->name, button.pin);

    k_timer_init(&pulse_timer, pulse_blue_color_step, NULL);
    hardware_led_off();

    return err;
}
