#ifndef ZIGBE_HH_APP
#define ZIGBE_HH_APP

#include "../main.h"
#include "zigbee_app_utils_op.h"


typedef struct {
    zigbee_app_rejoin_started_callback_t zigbee_app_rejoin_started_callback;
    zigbee_app_rejoin_stopped_callback_t zigbee_app_rejoin_stopped_callback;
} zigbee_app_rejoin_callbacks_t;

void zigbee_app_init(zigbee_app_rejoin_callbacks_t zigbee_app_rejoin_callbacks);

void zigbee_app_start();

void zigbee_app_factory_reset();

void zigbee_app_update(measurments_t measurements);

typedef void (*zigbee_app_joined_callback)(void);
typedef void (*zigbee_app_left_callback)(void);

// typedef void (*zigbee_app_rejoin_started_callback)(void);
// typedef void (*zigbee_app_rejoin_stopped_callback)(void);

#endif