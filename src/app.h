#ifndef APP_HH
#define APP_HH

#include <zephyr/kernel.h>
#include "validation.h"

#define INPUT_MIN_BATTERY 2000
#define INPUT_MAX_BATTERY 3000
#define OUTPUT_MIN_BATTERY 0
#define OUTPUT_MAX_BATTERY 1000

typedef void (*notify_api)(uint16_t soil_moisture);

struct notify_api
{
    notify_api send_notification;
};

typedef int (*read_adc_mv_moisture)(void);
typedef int (*read_adc_mv_battery)(void);


struct hardware_api
{
    read_adc_mv_moisture    read_adc_mv_moisture;
    read_adc_mv_battery     read_adc_mv_battery;
};

int app_init(struct notify_api * notify, struct hardware_api * hardware);
void app_main_loop(void);

#endif