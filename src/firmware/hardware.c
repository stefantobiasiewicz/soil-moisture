#include "hardware.h"

#include <math.h>



#include <zephyr/logging/log.h>

#include "../validation.h"

LOG_MODULE_REGISTER(hardware, LOG_LEVEL_DBG);

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

static const struct adc_dt_spec adc_battery_chan = ADC_DT_SPEC_GET_BY_IDX(ZEPHYR_USER_NODE, 0);
static const struct adc_dt_spec adc_soil_chan = ADC_DT_SPEC_GET_BY_IDX(ZEPHYR_USER_NODE, 1);
static const struct adc_dt_spec adc_temp_chan = ADC_DT_SPEC_GET_BY_IDX(ZEPHYR_USER_NODE, 2);
static const struct adc_dt_spec adc_vcc_chan = ADC_DT_SPEC_GET_BY_IDX(ZEPHYR_USER_NODE, 3);


const struct gpio_dt_spec power_high = GPIO_DT_SPEC_GET(DT_NODELABEL(power_high_out), gpios);
const struct gpio_dt_spec power_internal = GPIO_DT_SPEC_GET(DT_NODELABEL(power_internal_out), gpios);

const struct gpio_dt_spec eink_rst = GPIO_DT_SPEC_GET(DT_NODELABEL(eink_rst_out), gpios);
const struct gpio_dt_spec eink_busy = GPIO_DT_SPEC_GET(DT_NODELABEL(eink_busy_in), gpios);

const struct pwm_dt_spec generator = PWM_DT_SPEC_GET_BY_NAME(ZEPHYR_USER_NODE, generator);

/**
 * UX
*/
static const struct gpio_dt_spec button_left = GPIO_DT_SPEC_GET_OR(DT_ALIAS(button_left), gpios, {0});
static struct gpio_callback button_left_cb_data;

static const struct gpio_dt_spec button_right = GPIO_DT_SPEC_GET_OR(DT_ALIAS(button_right), gpios, {0});
static struct gpio_callback button_right_cb_data;

static struct hardware_callback_t callbacks;


static const struct pwm_dt_spec pwm_led_r = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led_r));
static const struct pwm_dt_spec pwm_led_g = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led_g));
static const struct pwm_dt_spec pwm_led_b = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led_b));


const struct i2c_dt_spec eink_1in9_com = I2C_DT_SPEC_GET(DT_NODELABEL(eink_1in9_com_i2c));
const struct i2c_dt_spec eink_1in9_data = I2C_DT_SPEC_GET(DT_NODELABEL(eink_1in9_data_i2c));



void hardware_genrator_on() {
    pwm_set_dt(&generator, 5000, 2500);
    LOG_INF("Generator turned ON: PWM 5000, duty cycle 2500");
}

void hardware_genrator_off() {
    pwm_set_dt(&generator, 5000, 0);
    LOG_INF("Generator turned OFF: PWM 5000, duty cycle 0");
}

void hardware_power_up() {
    gpio_pin_set_dt(&power_high, 1);
    LOG_INF("Power UP: power_high set to 1");
}

void hardware_power_down() {
    gpio_pin_set_dt(&power_high, 0);
    LOG_INF("Power DOWN: power_high set to 0");
}

void hardware_power_internal_up() {
    gpio_pin_set_dt(&power_internal, 1);
    LOG_INF("Internal Power UP: power_internal set to 1");
}

void hardware_power_internal_down() {
    gpio_pin_set_dt(&power_internal, 0);
    LOG_INF("Internal Power DOWN: power_internal set to 0");
}

void hardware_eink_rst_active() {
    gpio_pin_set_dt(&eink_rst, 0);
    LOG_INF("E-Ink Reset ACTIVE: eink_rst set to 0");
}

void hardware_eink_rst_inactive() {
    gpio_pin_set_dt(&eink_rst, 1);
    LOG_INF("E-Ink Reset INACTIVE: eink_rst set to 1");
}

int hardware_eink_busy_read() {
    int busy = gpio_pin_get_dt(&eink_busy);
    LOG_DBG("E-Ink Busy state read: %d", busy);
    return busy;
}



static struct k_timer pulse_timer;
static float time_step = 0;      
static float pulse_speed = 0.05; 
static uint8_t pulse_red = 0;    
static uint8_t pulse_green = 0;  
static uint8_t pulse_blue = 0;   

#define M_PI		3.14159265358979323846

static void led_pulse_step() {
    float brightness_factor = (sin(time_step) + 1) / 2; 

    uint8_t lr = (uint8_t)(pulse_red * brightness_factor);
    uint8_t lg = (uint8_t)(pulse_green * brightness_factor);
    uint8_t lb = (uint8_t)(pulse_blue * brightness_factor);

    hardware_set_led_color(lr, lg, lb);

    time_step += pulse_speed;

    if (time_step > 2 * M_PI) {
        time_step = 0;
    }
}

static void led_timer_end() {
    hardware_set_led_color(0,0,0);
}

void hardware_set_led_color(uint8_t red, uint8_t green, uint8_t blue) {
    pwm_set_pulse_dt(&pwm_led_r, PWM_USEC(red));
    pwm_set_pulse_dt(&pwm_led_g, PWM_USEC(green));
    pwm_set_pulse_dt(&pwm_led_b, PWM_USEC(blue));
}


void hardware_led_off(void) {
    k_timer_stop(&pulse_timer);
    hardware_set_led_color(0,0,0);
}

/**
 * timeout - K_MSEC(0)
 */
void hardware_led_pulse_start(uint8_t red, uint8_t green, uint8_t blue, k_timeout_t timeout) {
    pulse_red = red;
    pulse_green = green;
    pulse_blue = blue;

    k_timer_start(&pulse_timer,  K_MSEC(1), K_MSEC(1));
}


static int read_mv_from_adc(const struct adc_dt_spec * adc) {
    LOG_DBG("hardware: reading adc form chanel with id: [%d].", adc->channel_id);

    int err = 0;

    int16_t buf;
	struct adc_sequence sequence = {
		.oversampling = 16,
		.buffer = &buf,
		.buffer_size = sizeof(buf),
	};

    int32_t val_mv;

    (void)adc_sequence_init_dt(adc, &sequence);

    err = adc_read(adc->dev, &sequence);
    if (err < 0) {
        LOG_ERR("Could not read (%d)", err);
        return -1;
    } 

    /* conversion to mV may not be supported, skip if not */
    val_mv = buf;
    err = adc_raw_to_millivolts_dt(adc,
                        &val_mv);
    if (err < 0) {
        LOG_ERR(" (value in mV not available)");
        return -1;
    }

    LOG_INF("hardware: result: [%d].", val_mv);
    return val_mv;
}

int hardware_read_adc_mv_battery(void) {
    return read_mv_from_adc(&adc_battery_chan);
}

int hardware_read_adc_mv_moisture(void) {    
    return read_mv_from_adc(&adc_soil_chan);
}

int hardware_read_adc_mv_tempNTC(void) {
    return read_mv_from_adc(&adc_temp_chan);
}

int hardware_read_adc_mv_vcc(void) {
    return read_mv_from_adc(&adc_vcc_chan);
}

bool check_left_button_pressed() {
    LOG_DBG("checking left button state.");
    bool result = false;
    if (gpio_pin_get_dt(&button_left) >= 1) {
        result = true;
    }

    LOG_DBG("button state: %d", result);
    return result;
}

bool check_right_button_pressed() {
    LOG_DBG("checking right button state.");
    bool result = false;
    if (gpio_pin_get_dt(&button_right) >= 1) {
        result = true;
    }

    LOG_DBG("button state: %d", result);
    return result;
}




static void left_button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	LOG_INF("Button left pressed at %" PRIu32 "", k_cycle_get_32());

    callbacks.app_left_button_press();
}


static void right_button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	LOG_INF("Button right pressed at %" PRIu32 "", k_cycle_get_32());

    callbacks.app_right_button_press();
}



int hardware_init(struct hardware_callback_t * callbacks_p) {
    int err = is_pointer_null(callbacks_p);
    if (err != ERROR_OK) {
        LOG_ERR("callbacks is NULL.");
        return err;
    }

    err = is_pointer_null(callbacks_p->app_left_button_press);
    if (err != ERROR_OK) {
        LOG_ERR("callbacks_p->app_button_press is NULL.");
        return err;
    }
    callbacks.app_left_button_press = callbacks_p->app_left_button_press;

    err = is_pointer_null(callbacks_p->app_right_button_press);
    if (err != ERROR_OK) {
        LOG_ERR("callbacks_p->app_button_press is NULL.");
        return err;
    }
    callbacks.app_right_button_press = callbacks_p->app_right_button_press;


	if (!device_is_ready(generator.dev)) {
		LOG_ERR("Pwm for soil sensor device not ready");
		return -1;
	}

	if (!device_is_ready(power_high.port)) {
		LOG_ERR("Gpio for poerw high device not ready");
		return -1;
	}

	err = gpio_pin_configure_dt(&power_high, GPIO_OUTPUT);
	if (err < 0) {
		LOG_ERR("Could not setup gpio for poerw high (%d)", err);
		return -1;
	}

	if (!device_is_ready(power_internal.port)) {
		LOG_ERR("Gpio for power internal device not ready");
		return -1;
	}

	err = gpio_pin_configure_dt(&power_internal, GPIO_OUTPUT);
	if (err < 0) {
		LOG_ERR("Could not setup gpio for power internal (%d)", err);
		return -1;
	}

	if (!device_is_ready(eink_rst.port)) {
		LOG_ERR("Gpio for eink rst internal device not ready");
		return -1;
	}

	err = gpio_pin_configure_dt(&eink_rst, GPIO_OUTPUT);
	if (err < 0) {
		LOG_ERR("Could not setup gpio for eink rst (%d)", err);
		return -1;
	}

    if (!device_is_ready(eink_busy.port)) {
		LOG_ERR("Gpio for eink busy internal device not ready");
		return -1;
	}

	err = gpio_pin_configure_dt(&eink_busy, GPIO_INPUT);
	if (err < 0) {
		LOG_ERR("Could not setup gpio for eink busy (%d)", err);
		return -1;
	}

    hardware_power_down();
    
    /**
     * 
     * adc channels config
     * 
    */
   	if (!device_is_ready(adc_battery_chan.dev)) {
		LOG_ERR("ADC controller for battery device not ready");
		return -1;
	}

	err = adc_channel_setup_dt(&adc_battery_chan);
	if (err < 0) {
		LOG_ERR("Could not setup adc battery channel (%d)", err);
		return -1;
	}

	if (!device_is_ready(adc_soil_chan.dev)) {
		LOG_ERR("ADC controller for soil device not ready");
		return -1;
	}

	err = adc_channel_setup_dt(&adc_soil_chan);
	if (err < 0) {
		LOG_ERR("Could not setup adc soil channel (%d)", err);
		return -1;
	}

	if (!device_is_ready(adc_temp_chan.dev)) {
		LOG_ERR("ADC controller for temperature NTC device not ready");
		return -1;
	}

	err = adc_channel_setup_dt(&adc_temp_chan);
	if (err < 0) {
		LOG_ERR("Could not setup adc temperature NTC channel (%d)", err);
		return -1;
	}

	if (!device_is_ready(adc_vcc_chan.dev)) {
		LOG_ERR("ADC controller for vcc device not ready");
		return -1;
	}

	err = adc_channel_setup_dt(&adc_vcc_chan);
	if (err < 0) {
		LOG_ERR("Could not setup adc vcc channel (%d)", err);
		return -1;
	}

    /**
     * 
     * pwm led config
     * 
    */
   	LOG_INF("initializing rgb led module");
   	if (!device_is_ready(pwm_led_r.dev)) {
		LOG_ERR("Error: PWM device %s is not ready",
		       pwm_led_r.dev->name);
		return -1;
	}


   	if (!device_is_ready(pwm_led_g.dev)) {
		LOG_ERR("Error: PWM device %s is not ready",
		       pwm_led_g.dev->name);
		return -1;
	}


   	if (!device_is_ready(pwm_led_b.dev)) {
		LOG_ERR("Error: PWM device %s is not ready",
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
   if (!device_is_ready(button_left.port)) {
		LOG_ERR("Error: button device %s is not ready",
		       button_left.port->name);
		return -1;
	}

	err = gpio_pin_configure_dt(&button_left, GPIO_INPUT);
	if (err != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d",
		       err, button_left.port->name, button_left.pin);
		return -1;
	}

	err = gpio_pin_interrupt_configure_dt(&button_left,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (err != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			err, button_left.port->name, button_left.pin);
		return -1;
	}

	gpio_init_callback(&button_left_cb_data, left_button_pressed, BIT(button_left.pin));
	gpio_add_callback(button_left.port, &button_left_cb_data);
	LOG_INF("Set up button at %s pin %d", button_left.port->name, button_left.pin);


   if (!device_is_ready(button_right.port)) {
		LOG_ERR("Error: button device %s is not ready",
		       button_right.port->name);
		return -1;
	}

	err = gpio_pin_configure_dt(&button_right, GPIO_INPUT);
	if (err != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d",
		       err, button_right.port->name, button_right.pin);
		return -1;
	}

	err = gpio_pin_interrupt_configure_dt(&button_right,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (err != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			err, button_right.port->name, button_right.pin);
		return -1;
	}

	gpio_init_callback(&button_right_cb_data, right_button_pressed, BIT(button_right.pin));
	gpio_add_callback(button_right.port, &button_right_cb_data);
	LOG_INF("Set up button at %s pin %d", button_right.port->name, button_right.pin);

	/**
	 * i2c uC decvices init
	 */
	if (!device_is_ready(eink_1in9_com.bus)) {
		LOG_ERR("I2C bus %s is not ready!\n\r",eink_1in9_com.bus->name);
		return -1;
	}

	if (!device_is_ready(eink_1in9_data.bus)) {
		LOG_ERR("I2C bus %s is not ready!\n\r",eink_1in9_data.bus->name);
		return -1;
	}


    /**
     * timer init
     */
    k_timer_init(&pulse_timer, led_pulse_step, led_timer_end);
    hardware_led_off();



	/**
	 * init states 
	 */
	hardware_eink_rst_inactive();
	hardware_genrator_off();
	hardware_power_down();
	hardware_power_internal_down();

    return err;
}
