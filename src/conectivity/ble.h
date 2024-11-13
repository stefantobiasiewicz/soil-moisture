#ifndef BLE_HH_S
#define BLE_HH_S

#include <zephyr/kernel.h>
#include "../main.h"



#define BT_UUID_SOIL_MOISTURE_SERVICE_ENCODED   BT_UUID_128_ENCODE(0x0000AAAA, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB)
#define BT_UUID_SOIL_MOISTURE_SERVICE           BT_UUID_DECLARE_128(BT_UUID_SOIL_MOISTURE_SERVICE_ENCODED)

#define BT_UUID_COMMANDS_ENCODED             BT_UUID_128_ENCODE(0x0000AAAA, 0x0001, 0x1000, 0x8000, 0x00805F9B34FB)
#define BT_UUID_COMMANDS                     BT_UUID_DECLARE_128(BT_UUID_COMMANDS_ENCODED)



/**
 * calibration API
*/
typedef void (*app_calibrate_t)(uint16_t command);

/**
 * sleep time API
*/
typedef void (*app_set_sleep_time_t)(uint16_t seconds);
typedef uint16_t (*app_get_sleep_time_t)(void);
/**
 * conection API
*/
typedef void (*app_connected_t)(void);
typedef void (*app_disconnected_t)(void);

typedef struct
{
    app_connected_t app_connected;
    app_disconnected_t app_disconnected;
} application_api_t;

int ble_init(application_api_t * api);

/**
 * advetising for connection and setup parameters
*/
int ble_advertise_connection_start();
int ble_advertise_connection_stop();

/**
 * advertising soil and battery data
*/
int ble_advertise_not_connection_data_start(measurments_t data);
int ble_advertise_not_connection_data_stop();

/**
 * notifiing soil and battery data when connection
*/
int ble_send_notify(uint16_t soil_value, uint16_t battery_value);



#endif
