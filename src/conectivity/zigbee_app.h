#ifndef ZIGBE_HH_APP
#define ZIGBE_HH_APP

#include "../main.h"

void zigbee_app_init();

void zigbee_app_start();

void zigbee_app_factory_reset();

void zigbee_app_update(measurments_t measurements);

typedef void (*zigbee_app_joined_callback)(void);
typedef void (*zigbee_app_left_callback)(void);

typedef void (*zigbee_app_rejoin_started_callback)(void);
typedef void (*zigbee_app_rejoin_stopped_callback)(void);

#endif