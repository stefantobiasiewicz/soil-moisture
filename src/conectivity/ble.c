#include "ble.h"


#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>
#include "../validation.h"


LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

#define APP_VERSION 0x0005


static ssize_t bt_commands_read(struct bt_conn *conn,
					    const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset);		

static ssize_t bt_commands_write(struct bt_conn *conn,
					     const struct bt_gatt_attr *attr,
					     const void *buf, uint16_t len,
					     uint16_t offset, uint8_t flags);


static void soil_moisture_service_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				  uint16_t value);

static ssize_t bt_soil_moisture_read(struct bt_conn *conn,
					    const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset);		

static ssize_t bt_battery_read(struct bt_conn *conn,
					    const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset);		
																		 
														 
static void soil_moisture_service_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				  uint16_t value);

static ssize_t bt_soil_moisture_read(struct bt_conn *conn,
					    const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset);		

static ssize_t bt_battery_read(struct bt_conn *conn,
					    const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset);		
																		 

BT_GATT_SERVICE_DEFINE(soil_moisture_service,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_SOIL_MOISTURE_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_COMMANDS,
					(BT_GATT_CHRC_WRITE | BT_GATT_CHRC_READ),
					(BT_GATT_PERM_WRITE | BT_GATT_PERM_READ),
					bt_commands_read,
					bt_commands_write,
					NULL)
);

struct bt_conn *my_conn = NULL;
static application_api_t application;

static bool notification_enabled = false;

typedef struct adv_mfg_data {
	uint16_t version;	    /* Company Identifier Code. */
	uint16_t ble_soil_moisture_value;
	uint16_t ble_battery_value;
	uint16_t unique_id;
} manufacture_data_t;

static manufacture_data_t adv_mfg_data = {0x0005, 0x0000, 0x0000, 0x0000};

typedef struct {
	uint16_t uuid;	
	uint16_t version;
	uint16_t ble_soil_moisture_value;
	uint16_t ble_battery_value;
	uint16_t unique_id;
} service_data_t;

static service_data_t service_data = {0x55aa, 0x0006, 0x0001, 0x0003, 0x0005};




/*advertising packet for connection*/
static const struct bt_data ad_connection[] = {
	/* STEP 4.1.2 - Set the advertising flags */    
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	/* STEP 4.1.3 - Set the advertising packet data  */
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

/*advertising packet wuth data*/
static const struct bt_data ad_data[] = {
	/* STEP 4.1.2 - Set the advertising flags */    
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &adv_mfg_data, sizeof(adv_mfg_data)),
	BT_DATA(BT_DATA_SVC_DATA16, &service_data, sizeof(service_data)),
};


/*scan response*/
static const struct bt_data sd[] = {
        /* 4.2.3 Include the URL data in the scan response packet*/
        BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_SOIL_MOISTURE_SERVICE_ENCODED),
};



static struct bt_le_adv_param *adv_param_connection =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE,
	800,
	801,
	NULL);    

static struct bt_le_adv_param *adv_param_data =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY,
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

	application.app_connected();
    
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
	application.app_disconnected();
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

    return ERROR_OK;
}


static ssize_t bt_commands_write(struct bt_conn *conn,
					     const struct bt_gatt_attr *attr,
					     const void *buf, uint16_t len,
					     uint16_t offset, uint8_t flags) {
	LOG_INF("ble: writing command.");

	uint16_t val = *((uint16_t *)buf);

	LOG_INF("ble: command: %d.", val);

	// application.app_set_sleep_time(val);

	return len;
}

static ssize_t bt_commands_read(struct bt_conn *conn,
					    const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset) {
	LOG_INF("ble: reading time interval.");
	
	uint16_t value = 0xaa;//application.app_get_sleep_time();
	LOG_INF("ble: seconds: %d.", value);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}



int ble_init(application_api_t * api) {
    LOG_INF("BLE initializing.");
   
    int err = is_pointer_null(api);
    if (err != ERROR_OK) {
        LOG_ERR("application_api is NULL.");
        return err;
    }
    err = is_pointer_null(api->app_connected);
    if (err != ERROR_OK) {
        LOG_ERR("api->app_connected is NULL.");
        return err;
    }
    err = is_pointer_null(api->app_disconnected);
    if (err != ERROR_OK) {
        LOG_ERR("api->app_disconnected is NULL.");
        return err;
    }


    application.app_connected = api->app_connected;
    application.app_disconnected = api->app_disconnected;

    return init_radio();
}

K_SEM_DEFINE(avetising_sem, 1, 1);

int ble_advertise_connection_start() {
	int err = bt_le_adv_stop();

	if (k_sem_take(&avetising_sem, K_FOREVER) == 0) {
		LOG_INF("ble: start advetising connection");
		int err = bt_le_adv_start(adv_param_connection, ad_connection, ARRAY_SIZE(ad_connection),
					sd, ARRAY_SIZE(sd));
		if (err) {
			LOG_ERR("Advertising failed to start (err %d)\n", err);
			return err;
		}
	}

	return ERROR_OK;
}

int ble_advertise_connection_stop() {
	LOG_INF("ble: stop advetising connection");
	int err = bt_le_adv_stop();
	
	k_sem_give(&avetising_sem);

	return err;
}


int ble_advertise_not_connection_data_start(measure_data_t data, uint16_t id) {
	LOG_INF("ble: setting battery value: [%d] and soil value: [%d] and unique_id: [%d].", data.battery_value, data.soil_value, id);
	adv_mfg_data.ble_battery_value = data.battery_value;
	adv_mfg_data.ble_soil_moisture_value = data.soil_value;
	adv_mfg_data.unique_id = id;

	if (k_sem_take(&avetising_sem, K_FOREVER) == 0) {
		LOG_INF("ble: start advetising soil data");
		int err = bt_le_adv_start(adv_param_data, ad_data, ARRAY_SIZE(ad_data),
					sd, ARRAY_SIZE(sd));
		if (err) {
			LOG_ERR("Advertising failed to start (err %d)\n", err);
			return err;
		}
	}

	return ERROR_OK;
}

int ble_advertise_not_connection_data_stop() {
	LOG_INF("ble: stop advetising soil data");
	int err = bt_le_adv_stop();
	
	k_sem_give(&avetising_sem);

	return err;
}

