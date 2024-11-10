#include "display.h"

#include "EPD_2in13b_V4.h"
#include "DEV_Config.h"

#include <zephyr/kernel.h>
#include "firmware/hardware.h"
#include "validation.h"
#include <zephyr/logging/log.h>

#include "lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_LOGO_PLANTGUARD
#define LV_ATTRIBUTE_IMG_LOGO_PLANTGUARD
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_LOGO_PLANTGUARD uint8_t logo_plantguard_map[] = {
    0xff,
    0xff,
    0xff,
    0xff, /*Color of index 0*/
    0x00,
    0x00,
    0x00,
    0xff, /*Color of index 1*/

    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x7f,
    0xc0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x0f,
    0xff,
    0xf0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x1f,
    0x80,
    0x7c,
    0x00,
    0x00,
    0x00,
    0x00,
    0x18,
    0x00,
    0x0e,
    0x00,
    0x00,
    0x00,
    0x00,
    0x18,
    0x00,
    0x06,
    0x00,
    0x00,
    0x00,
    0x00,
    0x0c,
    0x38,
    0x03,
    0x00,
    0x00,
    0x00,
    0x00,
    0x0c,
    0x3c,
    0x03,
    0x00,
    0x3f,
    0xc0,
    0x00,
    0x0c,
    0x0f,
    0x01,
    0x81,
    0xff,
    0xfc,
    0x00,
    0x06,
    0x03,
    0x81,
    0x83,
    0xc0,
    0x3e,
    0x00,
    0x06,
    0x01,
    0xe1,
    0x87,
    0x00,
    0x06,
    0x00,
    0x03,
    0x00,
    0x71,
    0x86,
    0x00,
    0x0e,
    0x00,
    0x03,
    0x80,
    0x39,
    0x8c,
    0x0f,
    0x0c,
    0x00,
    0x01,
    0x80,
    0x1f,
    0x8c,
    0x7e,
    0x0c,
    0x00,
    0x00,
    0xc0,
    0x0f,
    0x0c,
    0xf0,
    0x18,
    0x00,
    0x00,
    0xe0,
    0x07,
    0x1f,
    0xc0,
    0x38,
    0x00,
    0x00,
    0x78,
    0x03,
    0x1f,
    0x00,
    0x30,
    0x00,
    0x00,
    0x1f,
    0xf9,
    0x9e,
    0x00,
    0x60,
    0x00,
    0x00,
    0x07,
    0xf1,
    0xb8,
    0x00,
    0xe0,
    0x00,
    0x00,
    0x00,
    0x00,
    0xf3,
    0x03,
    0xc0,
    0x00,
    0x00,
    0x00,
    0x00,
    0xe3,
    0xff,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xe1,
    0xfc,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xc0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xc0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xc0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x3f,
    0xff,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x3f,
    0xff,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x00,
    0x3f,
    0xff,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x3f,
    0xff,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x0c,
    0x00,
    0x00,
    0x0c,
    0x00,
    0x00,
    0x00,
    0x06,
    0x00,
    0x00,
    0x18,
    0x00,
    0x00,
    0x00,
    0x06,
    0x00,
    0x00,
    0x18,
    0x00,
    0x00,
    0x00,
    0x06,
    0x00,
    0x00,
    0x18,
    0x00,
    0x00,
    0x00,
    0x06,
    0x00,
    0x00,
    0x18,
    0x00,
    0x00,
    0x00,
    0x06,
    0x00,
    0x00,
    0x18,
    0x00,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x00,
    0x03,
    0x00,
    0x00,
    0x30,
    0x00,
    0x00,
    0x00,
    0x01,
    0x80,
    0x00,
    0x60,
    0x00,
    0x00,
    0x00,
    0x01,
    0xff,
    0xff,
    0xe0,
    0x00,
    0x00,
    0x00,
    0x01,
    0xff,
    0xff,
    0xe0,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

const lv_img_dsc_t logo_plantguard = {
    .header.cf = LV_IMG_CF_INDEXED_1BIT,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 50,
    .header.h = 50,
    .data_size = 358,
    .data = logo_plantguard_map,
};

struct epd_display_fn_t
{
  void (*epd_init)(void);
  void (*epd_clear)(void);
  void (*epd_display_full)(uint8_t *Image, ...);
  void (*epd_write_display)(uint16_t X_start, uint16_t Y_start, uint16_t width,
                            uint16_t height, uint8_t *Image);
  void (*epd_turn_on_display)(void);
  void (*epd_sleep)(void);
} epd_display_fn;

void display_init()
{
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
  // epd_display_fn.epd_clear();
}


static int get_integer_part(float value)
{
  return (int)value;
}

// Funkcja zwracająca część dziesiętną (1 miejsce po przecinku)
static int get_decimal_part(float value)
{
  int integer_part = (int)value;
  int decimal_part = (int)((value - integer_part) * 10);
  if (decimal_part < 0)
    decimal_part *= -1; // Obsługa wartości ujemnych
  return decimal_part;
}

void main_screen(measurments_t measuremet)
{
  lv_obj_t *screen = lv_scr_act();
  lv_obj_set_size(screen, lv_pct(100), lv_pct(100));
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

  static lv_style_t style_border;
  lv_style_init(&style_border);
  lv_style_set_border_color(&style_border, lv_color_black());
  lv_style_set_border_width(&style_border, 0);
  lv_style_set_pad_all(&style_border, 1);
  //   lv_style_set_margin_all(&style_border, 0);

  lv_obj_t *div_1 = lv_obj_create(screen);
  lv_obj_set_size(div_1, lv_pct(100), lv_pct(7));
  lv_obj_set_pos(div_1, lv_pct(0), lv_pct(0));
  lv_obj_add_style(div_1, &style_border, LV_PART_MAIN);
  lv_obj_clear_flag(div_1, LV_OBJ_FLAG_SCROLLABLE);
  //   lv_obj_set_style_bg_color(div_1, lv_palette_main(LV_PALETTE_RED), _LV_STYLE_STATE_CMP_SAME);

  lv_obj_t *battery_icon = lv_label_create(div_1);
  lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_1);
  lv_obj_set_align(battery_icon, LV_ALIGN_RIGHT_MID);

  lv_obj_t *div_2 = lv_obj_create(screen);
  lv_obj_set_size(div_2, lv_pct(100), lv_pct(50));
  lv_obj_set_pos(div_2, lv_pct(0), lv_pct(7));
  lv_obj_add_style(div_2, &style_border, LV_PART_MAIN);
  lv_obj_clear_flag(div_2, LV_OBJ_FLAG_SCROLLABLE);
  //   lv_obj_set_style_bg_color(div_2, lv_palette_main(LV_PALETTE_BLUE), _LV_STYLE_STATE_CMP_SAME);

  lv_obj_t *arc_back = lv_arc_create(div_2);
  lv_obj_set_size(arc_back, lv_pct(100), lv_pct(100));
  lv_arc_set_rotation(arc_back, 140);
  lv_obj_set_style_arc_width(arc_back, 14, LV_PART_MAIN);
  lv_arc_set_bg_angles(arc_back, 0, 260);
  lv_obj_remove_style(arc_back, NULL, LV_PART_KNOB);
  lv_arc_set_value(arc_back, 100);
  // lv_obj_align(arc_back, LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_style_arc_color(arc_back, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(arc_back, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);

  lv_obj_t *arc = lv_arc_create(div_2);
  lv_obj_set_size(arc, lv_pct(100), lv_pct(100));
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
  lv_arc_set_value(arc, 20);
  lv_obj_center(arc);
  lv_obj_set_style_arc_color(arc, lv_color_black(), LV_PART_INDICATOR);
  // lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);

  lv_obj_t *arc_label = lv_label_create(div_2);
  lv_label_set_text(arc_label, "20%");
  lv_obj_center(arc_label);

  static lv_style_t arc_label_style;
  lv_style_init(&arc_label_style);
  lv_style_set_text_font(&arc_label_style, &lv_font_montserrat_28); // Bigger font
  lv_obj_add_style(arc_label, &arc_label_style, LV_PART_MAIN);

  lv_obj_t *div_3 = lv_obj_create(screen);
  lv_obj_set_size(div_3, lv_pct(100), lv_pct(20));
  lv_obj_set_pos(div_3, lv_pct(0), lv_pct(50 + 7));
  lv_obj_add_style(div_3, &style_border, LV_PART_MAIN);
  lv_obj_clear_flag(div_3, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_side(div_3, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
  lv_obj_set_style_border_width(div_3, 2, LV_PART_MAIN);
  //   lv_obj_set_style_bg_color(div_3, lv_palette_main(LV_PALETTE_RED), _LV_STYLE_STATE_CMP_SAME);

  lv_obj_t *gnd_temp_label = lv_label_create(div_3);
  lv_label_set_text(gnd_temp_label, "15.5°C");
  lv_obj_align(gnd_temp_label, LV_ALIGN_TOP_MID, 0, 0);

  static lv_style_t gnd_temp_label_style;
  lv_style_init(&gnd_temp_label_style);
  lv_style_set_text_font(&gnd_temp_label_style, &lv_font_montserrat_28); // Bigger font
  lv_obj_add_style(gnd_temp_label, &gnd_temp_label_style, LV_PART_MAIN);

  lv_obj_t *div_4 = lv_obj_create(screen);
  lv_obj_set_size(div_4, lv_pct(50), lv_pct(12));
  lv_obj_set_pos(div_4, lv_pct(0), lv_pct(20 + 50 + 7));
  lv_obj_add_style(div_4, &style_border, LV_PART_MAIN);
  lv_obj_clear_flag(div_4, LV_OBJ_FLAG_SCROLLABLE);
  //   lv_obj_set_style_bg_color(div_4, lv_palette_main(LV_PALETTE_BLUE), _LV_STYLE_STATE_CMP_SAME);

  lv_obj_t *lux_label = lv_label_create(div_4);
  lv_label_set_text(lux_label, "57000Lx");
  lv_obj_align(lux_label, LV_ALIGN_LEFT_MID, 0, 0);

  static lv_style_t lux_label_style;
  lv_style_init(&lux_label_style);
  lv_style_set_text_font(&lux_label_style, &lv_font_montserrat_16); // Bigger font
  lv_obj_add_style(lux_label, &lux_label_style, LV_PART_MAIN);

  lv_obj_t *div_5 = lv_obj_create(screen);
  lv_obj_set_size(div_5, lv_pct(50), lv_pct(12));
  lv_obj_set_pos(div_5, lv_pct(50), lv_pct(20 + 50 + 7));
  lv_obj_add_style(div_5, &style_border, LV_PART_MAIN);
  lv_obj_clear_flag(div_5, LV_OBJ_FLAG_SCROLLABLE);
  //   lv_obj_set_style_bg_color(div_5, lv_palette_main(LV_PALETTE_BLUE), _LV_STYLE_STATE_CMP_SAME);

  lv_obj_t *air_temp_label = lv_label_create(div_5);
  lv_label_set_text(air_temp_label, "23.5°C");
  lv_obj_align(air_temp_label, LV_ALIGN_RIGHT_MID, 0, 0);

  static lv_style_t air_temp_label_style;
  lv_style_init(&air_temp_label_style);
  lv_style_set_text_font(&air_temp_label_style, &lv_font_montserrat_16); // Bigger font
  lv_obj_add_style(air_temp_label, &air_temp_label_style, LV_PART_MAIN);

  lv_obj_t *div_6 = lv_obj_create(screen);
  lv_obj_set_size(div_6, lv_pct(100), lv_pct(10));
  lv_obj_set_pos(div_6, lv_pct(0), lv_pct(20 + 50 + 7 + 12));
  lv_obj_add_style(div_6, &style_border, LV_PART_MAIN);
  lv_obj_clear_flag(div_6, LV_OBJ_FLAG_SCROLLABLE);
  //   lv_obj_set_style_bg_color(div_6, lv_palette_main(LV_PALETTE_BLUE), _LV_STYLE_STATE_CMP_SAME);

  lv_obj_t *air_hum_label = lv_label_create(div_6);
  lv_label_set_text(air_hum_label, "23.5% Rh");
  lv_obj_center(air_hum_label);

  static lv_style_t air_hum_label_style;
  lv_style_init(&air_hum_label_style);
  lv_style_set_text_font(&air_hum_label_style, &lv_font_montserrat_16); // Bigger font
  lv_obj_add_style(air_hum_label, &air_hum_label_style, LV_PART_MAIN);

  /**
   * setting all veriables
   */

  if (measuremet.battery > 90)
  {
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
  }
  else if (measuremet.battery > 70)
  {
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_3);
  }
  else if (measuremet.battery > 45)
  {
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_2);
  }
  else if (measuremet.battery > 20)
  {
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_1);
  }
  else
  {
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_EMPTY);
  }

  int moisture_int = get_integer_part(measuremet.soil_moisture);
  int moisture_dec = get_decimal_part(measuremet.soil_moisture);
  lv_arc_set_value(arc, moisture_int);
  if (moisture_int < 20)
  {
    lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
  }
  lv_label_set_text_fmt(arc_label, "%d.%d%%", moisture_int, moisture_dec);

  int gnd_temp_int = get_integer_part(measuremet.temperature_ground);
  int gnd_temp_dec = get_decimal_part(measuremet.temperature_ground);
  lv_label_set_text_fmt(gnd_temp_label, "%d.%d°C", gnd_temp_int, gnd_temp_dec);

  int air_temp_int = get_integer_part(measuremet.air_temperature);
  int air_temp_dec = get_decimal_part(measuremet.air_temperature);
  lv_label_set_text_fmt(air_temp_label, "%d.%d°C", air_temp_int, air_temp_dec);

  int air_hum_int = get_integer_part(measuremet.air_humidity);
  int air_hum_dec = get_decimal_part(measuremet.air_humidity);
  lv_label_set_text_fmt(air_hum_label, "%d.%d%%Rh", air_hum_int, air_hum_dec);

  // Wartości całkowite mogą pozostać bez zmian
  lv_label_set_text_fmt(lux_label, "%dLx", (int)measuremet.lux);
}

void display_values(measurments_t measuremet)
{
  lv_obj_clean(lv_scr_act());
  main_screen(measuremet);
  lv_task_handler();
}

void display_clean()
{
  epd_display_fn.epd_clear();
}
