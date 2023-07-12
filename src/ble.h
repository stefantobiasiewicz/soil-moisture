#ifndef BLE_HH_S
#define BLE_HH_S

#include <zephyr/kernel.h>



#define BT_UUID_SOIL_MOISTURE_SERVICE_ENCODED   BT_UUID_128_ENCODE(0x0000AAAA, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB)
#define BT_UUID_SOIL_MOISTURE_SERVICE           BT_UUID_DECLARE_128(BT_UUID_SOIL_MOISTURE_SERVICE_ENCODED)

#define BT_UUID_CALIBRATION_ENCODED             BT_UUID_128_ENCODE(0x0000AAAA, 0x0001, 0x1000, 0x8000, 0x00805F9B34FB)
#define BT_UUID_CALIBRATION                     BT_UUID_DECLARE_128(BT_UUID_CALIBRATION_ENCODED)

#define BT_UUID_TIME_INTERVAL_ENCODED           BT_UUID_128_ENCODE(0x0000AAAA, 0x0002, 0x1000, 0x8000, 0x00805F9B34FB)
#define BT_UUID_TIME_INTERVAL                   BT_UUID_DECLARE_128(BT_UUID_TIME_INTERVAL_ENCODED)

#define BT_UUID_SOIL_MOISTURE_ENCODED           BT_UUID_128_ENCODE(0x0000AAAA, 0x0003, 0x1000, 0x8000, 0x00805F9B34FB)
#define BT_UUID_SOIL_MOISTURE                   BT_UUID_DECLARE_128(BT_UUID_SOIL_MOISTURE_ENCODED)

#define BT_UUID_BATTERY_LEVEL_ENCODED           BT_UUID_128_ENCODE(0x0000AAAA, 0x0004, 0x1000, 0x8000, 0x00805F9B34FB)
#define BT_UUID_BATTERY_LEVEL                   BT_UUID_DECLARE_128(BT_UUID_BATTERY_LEVEL_ENCODED)



/**
 * calibration API
*/
typedef void (*app_calibrate_t)(uint16_t command);

/**
 * notification time API
*/
typedef void (*app_set_notification_time_t)(uint16_t seconds);
typedef uint16_t (*app_get_notification_time_t)(void);

struct application_api
{
    app_calibrate_t app_calibrate;

    app_set_notification_time_t app_set_notification_time;
    app_get_notification_time_t app_get_notification_time;
};


int ble_init(struct application_api * api);

int ble_send_notify(uint16_t soil_value, uint16_t battery_value);

#endif
