#include "ble.h"
#include "validation.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

struct bt_conn *my_conn = NULL;
static struct application_api application;

static bool notification_enabled = false;
static uint16_t ble_soil_moisture_value = 0;
static uint16_t ble_battery_value = 0;



/*advertising packet*/
static const struct bt_data ad[] = {
	/* STEP 4.1.2 - Set the advertising flags */    
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	/* STEP 4.1.3 - Set the advertising packet data  */
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};


/*scan response*/
static const struct bt_data sd[] = {
        /* 4.2.3 Include the URL data in the scan response packet*/
        BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_SOIL_MOISTURE_SERVICE_ENCODED),
};



static struct bt_le_adv_param *adv_param =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE,
	800,
	801,
	NULL);


/* ---BLE CONNECTION--- */
static void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    my_conn = bt_conn_ref(conn);
    
    struct bt_conn_info info;
    err = bt_conn_get_info(conn, &info);
    if (err) {
        LOG_ERR("bt_conn_get_info() returned %d", err);
        return;
    }
    uint16_t connection_interval = (uint16_t)info.le.interval*1.25; // in ms
    uint16_t supervision_timeout = info.le.timeout*10; // in ms
    LOG_INF("Connection parameters: interval %.2d ms, latency %d intervals, timeout %d ms", connection_interval, info.le.latency, supervision_timeout);
}
static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(my_conn);
}

static struct bt_conn_cb connection_callbacks = {
    .connected              = on_connected,
    .disconnected           = on_disconnected,
};


/* ---BLE CONNECTION--- */


static int init_radio() {
	int err;

	err = bt_enable(NULL); 
    if (err) {
        LOG_ERR("Bluetooth enabled failed (err %d)\n", err);
        return err;
    }
    LOG_INF("Bluetooth enabled\n");

	bt_addr_le_t addr;
    err = bt_addr_le_from_str("FF:EE:DD:CC:BB:AA", "random", &addr);
    if (err) {
        LOG_ERR("Invalid BT address (err %d)\n", err);
    }
    err = bt_id_create(&addr, NULL);
    if (err < 0) {
        LOG_ERR("Creating new ID failed (err %d)\n", err);
    }   

    bt_conn_cb_register(&connection_callbacks);

    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return err;
	}

    return ERROR_OK;
}


static ssize_t bt_calibration(struct bt_conn *conn,
					     const struct bt_gatt_attr *attr,
					     const void *buf, uint16_t len,
					     uint16_t offset, uint8_t flags) {
	LOG_INF("ble: calibration update.");

	uint16_t val = *((uint16_t *)buf);

	LOG_INF("ble: seconds: %d.", val);

	application.app_calibrate(val);

	return len;
}


static ssize_t bt_time_interval_write(struct bt_conn *conn,
					     const struct bt_gatt_attr *attr,
					     const void *buf, uint16_t len,
					     uint16_t offset, uint8_t flags) {
	LOG_INF("ble: writing time interval.");

	uint16_t val = *((uint16_t *)buf);

	LOG_INF("ble: seconds: %d.", val);

	application.app_set_notification_time(val);

	return len;
}

static ssize_t bt_time_interval_read(struct bt_conn *conn,
					     const struct bt_gatt_attr *attr,
					     const void *buf, uint16_t len,
					     uint16_t offset, uint8_t flags) {
	LOG_INF("ble: reading time interval.");
	
	uint16_t value = application.app_get_notification_time();
	LOG_INF("ble: seconds: %d.", value);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t bt_soil_moisture_read(struct bt_conn *conn,
					     const struct bt_gatt_attr *attr,
					     const void *buf, uint16_t len,
					     uint16_t offset, uint8_t flags) {
	LOG_INF("ble: reading soil moisture value: [%d].", ble_soil_moisture_value);
	
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &ble_soil_moisture_value, sizeof(ble_soil_moisture_value));
}

static ssize_t bt_battery_read(struct bt_conn *conn,
					     const struct bt_gatt_attr *attr,
					     const void *buf, uint16_t len,
					     uint16_t offset, uint8_t flags) {
	LOG_INF("ble: reading battery value: [%d].", ble_battery_value);
	
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &ble_battery_value, sizeof(ble_battery_value));
}

static void soil_moisture_service_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				  uint16_t value)
{
	notification_enabled = (value == BT_GATT_CCC_NOTIFY);
}


BT_GATT_SERVICE_DEFINE(soil_moisture_service,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_SOIL_MOISTURE_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_CALIBRATION,
					(BT_GATT_CHRC_WRITE),
					(BT_GATT_PERM_WRITE), 
					NULL, 
					bt_calibration,
					NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_TIME_INTERVAL,
					(BT_GATT_CHRC_WRITE | BT_GATT_CHRC_READ),
					(BT_GATT_PERM_WRITE | BT_GATT_PERM_READ),
					bt_time_interval_read,
					bt_time_interval_write,
					NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_SOIL_MOISTURE,
					(BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
					BT_GATT_PERM_READ,
					bt_soil_moisture_read,
					NULL,
					NULL),
    BT_GATT_CCC(soil_moisture_service_ccc_cfg_changed,
		            BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_BATTERY_LEVEL,
					BT_GATT_CHRC_READ,
					BT_GATT_PERM_READ,
					bt_battery_read,
					NULL,
					NULL),
);

int ble_send_notify(uint16_t soil_value)
{
	LOG_INF("sending notification with soil value: [%d].", (int) soil_value);

	ble_soil_moisture_value = soil_value;

	if (!notification_enabled) {
		LOG_WRN("notification not permited.");
		return -EACCES;
	}
	return bt_gatt_notify(NULL, &soil_moisture_service.attrs[6],
			      &soil_value,
			      sizeof(soil_value));
}

void ble_set_battery(uint16_t battery_value) {
	LOG_INF("setting battery value to ble module: [%d].", battery_value);

	ble_battery_value = battery_value;
}


int ble_init(struct application_api * api) {
    LOG_INF("BLE initializing.");
   
    int err = is_pointer_null(api);
    if (err != ERROR_OK) {
        LOG_ERR("application_api is NULL.");
        return err;
    }

    err = is_pointer_null(api->app_calibrate);
    if (err != ERROR_OK) {
        LOG_ERR("api->app_calibrate is NULL.");
        return err;
    }
    err = is_pointer_null(api->app_get_notification_time);
    if (err != ERROR_OK) {
        LOG_ERR("api->app_get_notification_time is NULL.");
        return err;
    }
    err = is_pointer_null(api->app_set_notification_time);
    if (err != ERROR_OK) {
        LOG_ERR("api->app_set_notification_time is NULL.");
        return err;
    }

    application.app_calibrate = api->app_calibrate;
    application.app_get_notification_time = api->app_get_notification_time;
    application.app_set_notification_time = api->app_set_notification_time;

    return init_radio();
}
