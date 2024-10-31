
/**
 * 
 * test to implement zephyr device driver
 * 
 * 
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>


struct dummy_display_config {
	uint16_t height;
	uint16_t width;
};

struct dummy_display_data {
	enum display_pixel_format current_pixel_format;
};


// Funkcja inicjalizacji displaya
static int my_display_init(const struct device *dev) {
    // Kod inicjalizacji twojego wyświetlacza

    struct dummy_display_data *disp_data = dev->data;

	disp_data->current_pixel_format = PIXEL_FORMAT_ARGB_8888;

	return 0;
}

static int dummy_display_write(const struct device *dev, const uint16_t x,
			       const uint16_t y,
			       const struct display_buffer_descriptor *desc,
			       const void *buf)
{
	const struct dummy_display_config *config = dev->config;

	__ASSERT(desc->width <= desc->pitch, "Pitch is smaller then width");
	__ASSERT(desc->pitch <= config->width,
		"Pitch in descriptor is larger than screen size");
	__ASSERT(desc->height <= config->height,
		"Height in descriptor is larger than screen size");
	__ASSERT(x + desc->pitch <= config->width,
		 "Writing outside screen boundaries in horizontal direction");
	__ASSERT(y + desc->height <= config->height,
		 "Writing outside screen boundaries in vertical direction");

	if (desc->width > desc->pitch ||
	    x + desc->pitch > config->width ||
	    y + desc->height > config->height) {
		return -EINVAL;
	}

	return 0;
}

static int dummy_display_blanking_off(const struct device *dev)
{
	return 0;
}

static int dummy_display_blanking_on(const struct device *dev)
{
	return 0;
}

static int dummy_display_set_brightness(const struct device *dev,
					const uint8_t brightness)
{
	return 0;
}

static int dummy_display_set_contrast(const struct device *dev,
				      const uint8_t contrast)
{
	return 0;
}

static void dummy_display_get_capabilities(const struct device *dev,
		struct display_capabilities *capabilities)
{
	const struct dummy_display_config *config = dev->config;
	struct dummy_display_data *disp_data = dev->data;

	capabilities->x_resolution = EPD_2IN13B_V4_WIDTH;
	capabilities->y_resolution = EPD_2IN13B_V4_HEIGHT;
	capabilities->supported_pixel_formats = PIXEL_FORMAT_MONO10 | PIXEL_FORMAT_MONO01;
	capabilities->current_pixel_format = PIXEL_FORMAT_MONO10;
	capabilities->screen_info = SCREEN_INFO_MONO_VTILED;
	capabilities->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

static int dummy_display_set_pixel_format(const struct device *dev,
		const enum display_pixel_format pixel_format)
{
	struct dummy_display_data *disp_data = dev->data;

	disp_data->current_pixel_format = pixel_format;
	return 0;
}

static const struct display_driver_api my_display_api = {
	.blanking_on = dummy_display_blanking_on,
	.blanking_off = dummy_display_blanking_off,
	.write = dummy_display_write,
	.set_brightness = dummy_display_set_brightness,
	.set_contrast = dummy_display_set_contrast,
	.get_capabilities = dummy_display_get_capabilities,
	.set_pixel_format = dummy_display_set_pixel_format,
};

#define EINK_DEFINE(node_id)                                                                   \
	DEVICE_DT_DEFINE(node_id, my_display_init, NULL, NULL, NULL, POST_KERNEL, CONFIG_DISPLAY_INIT_PRIORITY, &my_display_api);

DT_FOREACH_STATUS_OKAY(generic_epd, EINK_DEFINE)