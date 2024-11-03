#include "display.h"

#include "EPD_2in13b_V4.h"
#include "DEV_Config.h"

#include <zephyr/kernel.h>
#include "firmware/hardware.h"
#include "validation.h"
#include <zephyr/logging/log.h>

#include "lvgl.h"



struct epd_display_fn_t {
	void (*epd_init)(void);
	void (*epd_clear)(void);
	void (*epd_display_full)(uint8_t *Image, ...);
	void (*epd_write_display)(uint16_t X_start, uint16_t Y_start, uint16_t width,
				  uint16_t height, uint8_t *Image);
	void (*epd_turn_on_display)(void);
	void (*epd_sleep)(void);
} epd_display_fn;


void display_init() {
    #ifdef CONFIG_EPD_2IN13B_V4
		epd_display_fn.epd_init = EPD_2IN13B_V4_Init;
		epd_display_fn.epd_clear = EPD_2IN13B_V4_Clear;
		epd_display_fn.epd_display_full = EPD_2IN13B_V4_Display;
		epd_display_fn.epd_sleep = EPD_2IN13B_V4_Sleep;
    #else
        LOG_ERR("No EPD driver found");
		return -ENODEV;
    #endif

    DEV_Module_Init();

	epd_display_fn.epd_init();
}

void display_power_on() {

}




void demo_create(void) {
    // Create a screen
    lv_obj_t *screen = lv_scr_act();

    // Set screen size to 250x122
    lv_obj_set_size(screen, 250, 122);

    // Disable scrolling on the screen
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    // Create a border (using a style for border and padding)
    static lv_style_t border_style;
    lv_style_init(&border_style);
    lv_style_set_border_width(&border_style, 2);
    lv_style_set_border_color(&border_style, lv_color_black());
    lv_style_set_pad_all(&border_style, 10);  // Padding inside the border

    lv_obj_add_style(screen, &border_style, LV_PART_MAIN);

    // Create a container (list) to group the numbers
    lv_obj_t *list = lv_obj_create(screen);
    lv_obj_set_size(list, 250, 122);  // Set size for the list
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);  // Center it
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);  // Disable scroll
    lv_obj_add_style(list, &border_style, LV_PART_MAIN);  // Add border to the list

    // Style for the numbers
    static lv_style_t number_style;
    lv_style_init(&number_style);
    lv_style_set_text_font(&number_style, &lv_font_montserrat_26);  // Bigger font
    lv_style_set_border_width(&number_style, 2);
    lv_style_set_border_color(&number_style, lv_color_black());
    lv_style_set_pad_all(&number_style, 10);

    // Create label for the temperature value
    lv_obj_t *temp_label = lv_label_create(list);
    lv_label_set_text(temp_label, "25°C");
    lv_obj_add_style(temp_label, &number_style, LV_PART_MAIN);
    lv_obj_align(temp_label, LV_ALIGN_TOP_MID, 0, 0);

    // Create label for the humidity value
    lv_obj_t *humidity_label = lv_label_create(list);
    lv_label_set_text(humidity_label, "60%");
    lv_obj_add_style(humidity_label, &number_style, LV_PART_MAIN);
    lv_obj_align(humidity_label, LV_ALIGN_BOTTOM_MID, 0, 0);
}


void append_list_element(lv_obj_t * list, const char * text, int value) {
    static lv_style_t border_style; 
    lv_style_init(&border_style);
    lv_style_set_border_width(&border_style, 1);
    lv_style_set_border_color(&border_style, lv_color_black());
    lv_style_set_radius(&border_style, 10);
    lv_style_set_pad_all(&border_style, 1); 


    lv_obj_t *item_container = lv_obj_create(list);
    lv_obj_set_size(item_container, lv_pct(100), lv_pct(18));
    lv_obj_add_style(item_container, &border_style, LV_PART_MAIN);  // Add border


    // First line of text (e.g., Temperature)
    lv_obj_t *line1_label = lv_label_create(item_container);
    lv_label_set_text(line1_label, text);
    lv_obj_align(line1_label, LV_ALIGN_TOP_LEFT, 5, 0);

    lv_obj_t *line2_label = lv_label_create(item_container);
    lv_label_set_text_fmt(line2_label, "%d", value);
    lv_obj_align(line2_label, LV_ALIGN_BOTTOM_RIGHT, -5, 0);  
  } 


void lv_example_list_2(void)
{
    /*Create a list*/
    lv_obj_t * list1 = lv_list_create(lv_scr_act());
    lv_obj_set_size(list1, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_row(list1, 5, 0);

    /*Add buttons to the list*/
    append_list_element(list1, "Humidity", 78);
    append_list_element(list1, "Temperature", 23);
    append_list_element(list1, "aaaa", 23);
    append_list_element(list1, "ssss", 23);
    append_list_element(list1, "ostatni?", 23);
}

void display_values(float temperature, float humidity) {

lv_example_list_2();
 	lv_task_handler();
    EPD_2IN13B_V4_Display_Buffers();
	// memset(gImage_2in13b_V4b, 0xff, 4000);
	// memset(gImage_2in13b_V4r, 0xff, 4000);

	// memset(gImage_2in13b_V4b, 0x0f, 1);
	// memset(gImage_2in13b_V4r, 0xf0, 1);
    // epd_display_fn.epd_display_full(gImage_2in13b_V4b, gImage_2in13b_V4r);
}

void display_power_off() {

}
void display_clean() {
    epd_display_fn.epd_clear();
}


