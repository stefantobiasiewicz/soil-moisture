#include "zigbee_app.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(zigbee_app, LOG_LEVEL_DBG);

#ifndef CONFIG_ZIGBEE

void zigbee_app_app_init() {
    LOG_WRN("Zigbe is not builded into image set 'CONFIG_ZIGBEE' to enable it.");
}

void zigbee_start() {
    LOG_WRN("Zigbe is not builded into image set 'CONFIG_ZIGBEE' to enable it.");
}

void zigbee_app_stop() {
    LOG_WRN("Zigbe is not builded into image set 'CONFIG_ZIGBEE' to enable it.");
}

void zigbee_app_factory_reset() {
   LOG_WRN("Zigbe is not builded into image set 'CONFIG_ZIGBEE' to enable it.");
}


#endif /* CONFIG_ZIGBEE */


#ifdef CONFIG_ZIGBEE

#include <zboss_api.h>
#include <zigbee/zigbee_error_handler.h>
#include "zb_nrf_platform.h"
#include "zigbee_app_utils_op.h"
#include "zb_zcl_soil_moisture_measurements.h"

#include "zboss_api_addons.h"
#include "zb_zcl_pressure_measurement.h"


/**
 * 
 * ZCL liblary
 * https://zigbeealliance.org/wp-content/uploads/2021/10/07-5123-08-Zigbee-Cluster-Library.pdf
 * 
 */



/* Basic cluster attributes initial values. For more information, see section 3.2.2.2 of the ZCL specification. */
#define SENSOR_INIT_BASIC_APP_VERSION       01                                  /**< Version of the application software (1 byte). */
#define SENSOR_INIT_BASIC_STACK_VERSION     10                                  /**< Version of the implementation of the Zigbee stack (1 byte). */
#define SENSOR_INIT_BASIC_HW_VERSION        11                                  /**< Version of the hardware of the device (1 byte). */
#define SENSOR_INIT_BASIC_MANUF_NAME        "OpenCoded"                            /**< Manufacturer name (32 bytes). */
#define SENSOR_INIT_BASIC_MODEL_ID          "PlantGuard_V1"                       /**< Model number assigned by the manufacturer (32-bytes long string). */
#define SENSOR_INIT_BASIC_DATE_CODE         "20241114"                          /**< Date provided by the manufacturer of the device in ISO 8601 format (YYYYMMDD), for the first 8 bytes. The remaining 8 bytes are manufacturer-specific. */
#define SENSOR_INIT_BASIC_POWER_SOURCE      ZB_ZCL_BASIC_POWER_SOURCE_BATTERY /**< Type of power source or sources available for the device. For possible values, see section 3.2.2.2.8 of the ZCL specification. */
#define SENSOR_INIT_BASIC_LOCATION_DESC     "Office desk"                       /**< Description of the physical location of the device (16 bytes). You can modify it during the commisioning process. */
#define SENSOR_INIT_BASIC_PH_ENV            ZB_ZCL_BASIC_ENV_UNSPECIFIED        /**< Description of the type of physical environment. For possible values, see section 3.2.2.2.10 of the ZCL specification. */

// #define PLANT_GUARD_SENSOR_ENDPOINT               10                                  /**< Device endpoint. Used to receive light controlling commands. */


/** @brief Declare attribute list for Power Configuration cluster - server side
    @param attr_list - attribute list name
    @param voltage - pointer to variable to store BatteryVoltage attribute
    @param size - pointer to variable to store BatterySize attribute
    @param quantity - pointer to variable to store BatteryQuantity attribute
    @param rated_voltage - pointer to variable to store BatteryRatedVoltage attribute
    @param alarm_mask - pointer to variable to store BatteryAlarmMask attribute
    @param voltage_min_threshold - pointer to variable to store BatteryVoltageMinThreshold attribute
    @param battery_percentage_remaining - pointer to variable to store BatteryPercentageRemaining attribute
*/
#define ZB_ZCL_DECLARE_POWER_CONFIG_ATTRIB_LIST_EXTEND_WITH_PERCENTAGE(attr_list,                                                      \
    voltage, size, quantity, rated_voltage, alarm_mask, voltage_min_threshold, battery_percentage_remaining)                                  \
  ZB_ZCL_START_DECLARE_ATTRIB_LIST(attr_list)                                                                   \
  ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID((voltage),),                               \
  ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_SIZE_ID((size),),                                     \
  ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_QUANTITY_ID((quantity),),                             \
  ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_RATED_VOLTAGE_ID((rated_voltage),),                   \
  ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_ALARM_MASK_ID((alarm_mask ),),                        \
  ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_MIN_THRESHOLD_ID((voltage_min_threshold),),   \
  ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID((battery_percentage_remaining),),    \
  ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

typedef struct
{
    zb_uint8_t voltage;  
    zb_uint8_t size;
    zb_uint8_t quantity;
    zb_uint8_t rated_voltage;
    zb_uint8_t alarm_mask;
    zb_uint8_t voltage_min_threshold;
    zb_uint8_t battery_percentage_remaining;
} power_config_attr_t;


typedef struct
{
    zb_int16_t  measure_value;
    zb_int16_t  min_measure_value;
    zb_int16_t  max_measure_value;
    zb_uint16_t tolerance;
} zb_zcl_measurement_attrs_t;


/* Main application customizable context. Stores all settings and static values. */
typedef struct
{
    zb_zcl_basic_attrs_ext_t      basic_attr;
    zb_zcl_identify_attrs_t       identify_attr;
    zb_zcl_measurement_attrs_t    temp_attr;
    zb_zcl_measurement_attrs_t    temp_ground_attr;
    zb_zcl_measurement_attrs_t    iluminance_attr;
    zb_zcl_measurement_attrs_t    soil_moisture_attr;
    zb_zcl_measurement_attrs_t    humidity_attr; 
    power_config_attr_t           power_config_attr;
} sensor_device_ctx_t;

static sensor_device_ctx_t m_dev_ctx;
/**
 * 
 * Plant guard Zigbee zboss definitions
 * 
 * 
 */

#define ZB_DEVICE_VER_PLANT_GUARD       0    /**< Multisensor device version. */
#define PLANT_GUARD_GROUND_ENDPOINT 7

/** @brief Declares cluster list for the PLANT_GUARD_GROUND_ENDPOINT endpoint device.
 *
 *  @param cluster_list_name            Cluster list variable name.
 *  @param basic_attr_list              Attribute list for the Basic cluster.
 *  @param identify_attr_list           Attribute list for the Identify cluster.
 *  @param temp_measure_attr_list       Attribute list for the Temperature Measurement cluster.
 *  @param soil_moisture_attr_list
 */
#define ZB_DECLARE_PLANT_GUARD_GROUND_CLUSTER_LIST(                 \
      cluster_list_name,                                            \
      basic_attr_list,                                              \
      identify_attr_list,                                           \
      temp_measure_attr_list,                                       \
      soil_moisture_attr_list,                                      \
      power_config_attr_lis)                                        \
      zb_zcl_cluster_desc_t cluster_list_name[] =                   \
      {                                                             \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_IDENTIFY,                               \
          ZB_ZCL_ARRAY_SIZE(identify_attr_list, zb_zcl_attr_t),     \
          (identify_attr_list),                                     \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        ),                                                          \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_BASIC,                                  \
          ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t),        \
          (basic_attr_list),                                        \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        ),                                                          \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,                       \
          ZB_ZCL_ARRAY_SIZE(temp_measure_attr_list, zb_zcl_attr_t), \
          (temp_measure_attr_list),                                 \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        ),                                                          \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_SOIL_MOISTURE_MEASUREMENT,              \
          ZB_ZCL_ARRAY_SIZE(soil_moisture_attr_list, zb_zcl_attr_t),\
          (soil_moisture_attr_list),                                \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        ),                                                          \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_IDENTIFY,                               \
          0,                                                        \
          NULL,                                                     \
          ZB_ZCL_CLUSTER_CLIENT_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        ),                                                          \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_POWER_CONFIG,                           \
          ZB_ZCL_ARRAY_SIZE(power_config_attr_list, zb_zcl_attr_t), \
          (power_config_attr_list),                                 \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        )                                                           \
      }


#define ZB_ZCL_DECLARE_PLANT_GUARD_GROUND_DESC(ep_name, ep_id, in_clust_num, out_clust_num) \
  ZB_DECLARE_SIMPLE_DESC(in_clust_num, out_clust_num);                                \
  ZB_AF_SIMPLE_DESC_TYPE(in_clust_num, out_clust_num) simple_desc_##ep_name =         \
  {                                                                                   \
    ep_id,                                                                            \
    ZB_AF_HA_PROFILE_ID,                                                              \
    ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID,                                               \
    ZB_DEVICE_VER_PLANT_GUARD,                                                        \
    0,                                                                                \
    in_clust_num,                                                                     \
    out_clust_num,                                                                    \
    {                                                                                 \
      ZB_ZCL_CLUSTER_ID_BASIC,                                                        \
      ZB_ZCL_CLUSTER_ID_IDENTIFY,                                                     \
      ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,                                             \
      ZB_ZCL_CLUSTER_ID_SOIL_MOISTURE_MEASUREMENT,                                    \
      ZB_ZCL_CLUSTER_ID_IDENTIFY,                                                     \
      ZB_ZCL_CLUSTER_ID_POWER_CONFIG,                                                 \
    }                                                                                 \
  }

#define ZB_PLANT_GUARD_GROUND_REPORT_ATTR_COUNT  (3 + ZB_ZCL_POWER_CONFIG_REPORT_ATTR_COUNT)    /**< Number of attributes mandatory for reporting in the Temperature and Pressure Measurement cluster. */
#define ZB_PLANT_GUARD_GROUND_IN_CLUSTER_NUM     4                                 /**< Number of the input (server) clusters in the multisensor device. */
#define ZB_PLANT_GUARD_GROUND_CLUSTER_NUM        1                                    /**< Number of the output (client) clusters in the multisensor device. */


/** @brief Declares endpoint for the plantguard GROUND endpoint.
 *   
 *  @param ep_name          Endpoint variable name.
 *  @param ep_id            Endpoint ID.
 *  @param cluster_list     Endpoint cluster list.
 */

#define ZB_ZCL_DECLARE_PLANT_GUARD_GROUND_EP_DESC(ep_name, ep_id, cluster_list)              \
  ZB_ZCL_DECLARE_PLANT_GUARD_GROUND_DESC(ep_name,                                       \
      ep_id,                                                                      \
      ZB_PLANT_GUARD_GROUND_IN_CLUSTER_NUM,                                             \
      ZB_PLANT_GUARD_GROUND_CLUSTER_NUM);                                           \
  ZBOSS_DEVICE_DECLARE_REPORTING_CTX(reporting_info## ep_name,            \
                                     ZB_PLANT_GUARD_GROUND_REPORT_ATTR_COUNT);          \
  ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id,                                     \
      ZB_AF_HA_PROFILE_ID,                                                        \
      0,                                                                          \
      NULL,                                                                       \
      ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t),                     \
      cluster_list,                                                               \
      (zb_af_simple_desc_1_1_t*)&simple_desc_##ep_name,                           \
      ZB_PLANT_GUARD_GROUND_REPORT_ATTR_COUNT, reporting_info## ep_name, 0, NULL)


ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list, &m_dev_ctx.identify_attr.identify_time);

ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT(basic_attr_list,
                                     &m_dev_ctx.basic_attr.zcl_version,
                                     &m_dev_ctx.basic_attr.app_version,
                                     &m_dev_ctx.basic_attr.stack_version,
                                     &m_dev_ctx.basic_attr.hw_version,
                                     m_dev_ctx.basic_attr.mf_name,
                                     m_dev_ctx.basic_attr.model_id,
                                     m_dev_ctx.basic_attr.date_code,
                                     &m_dev_ctx.basic_attr.power_source,
                                     m_dev_ctx.basic_attr.location_id,
                                     &m_dev_ctx.basic_attr.ph_env,
                                     m_dev_ctx.basic_attr.sw_ver);


ZB_ZCL_DECLARE_TEMP_MEASUREMENT_ATTRIB_LIST(temperature_ground_attr_list, 
                                            &m_dev_ctx.temp_ground_attr.measure_value,
                                            &m_dev_ctx.temp_ground_attr.min_measure_value, 
                                            &m_dev_ctx.temp_ground_attr.max_measure_value, 
                                            &m_dev_ctx.temp_ground_attr.tolerance);


ZB_ZCL_DECLARE_SOIL_MOISTURE_MEASUREMENT_ATTRIB_LIST(soil_mositure_attr_list,
                                                    &m_dev_ctx.soil_moisture_attr.measure_value,
                                                    &m_dev_ctx.soil_moisture_attr.min_measure_value,
                                                    &m_dev_ctx.soil_moisture_attr.max_measure_value);

ZB_ZCL_DECLARE_POWER_CONFIG_ATTRIB_LIST_EXTEND_WITH_PERCENTAGE(power_config_attr_list,
                                             &m_dev_ctx.power_config_attr.voltage,
                                             &m_dev_ctx.power_config_attr.size,
                                             &m_dev_ctx.power_config_attr.quantity,
                                             &m_dev_ctx.power_config_attr.rated_voltage,
                                             &m_dev_ctx.power_config_attr.alarm_mask,
                                             &m_dev_ctx.power_config_attr.voltage_min_threshold,
                                             &m_dev_ctx.power_config_attr.battery_percentage_remaining);

ZB_DECLARE_PLANT_GUARD_GROUND_CLUSTER_LIST(plant_guard_ground_clusters,
                                     basic_attr_list,
                                     identify_attr_list,
                                     temperature_ground_attr_list,
                                     soil_mositure_attr_list, 
                                     power_config_attr_list);

ZB_ZCL_DECLARE_PLANT_GUARD_GROUND_EP_DESC(plant_guard_ground_ep,
                               PLANT_GUARD_GROUND_ENDPOINT,
                               plant_guard_ground_clusters);



/**
 * 
 * Air sensing endpoint
 * 
 */

#define PLANT_GUARD_AIR_ENDPOINT 8

/** @brief Declares cluster list for the multisensor device.
 *
 *  @param cluster_list_name            Cluster list variable name.
 *  @param temp_measure_attr_list       Attribute list for the Temperature Measurement cluster.
 *  @param iluminance_measure_attr_list     Attribute list for the iluminance Measurement cluster. 
 *  @param humi_meassure_attr_list      Attribute list for the Rel Humidity Measurement cluster
 */
#define ZB_DECLARE_PLANT_GUARD_AIR_CLUSTER_LIST(                    \
      cluster_list_name,                                            \
      temp_measure_attr_list,                                       \
      iluminance_measure_attr_list,                                 \
      humi_meassure_attr_list)                                      \
      zb_zcl_cluster_desc_t cluster_list_name[] =                   \
      {                                                             \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,                       \
          ZB_ZCL_ARRAY_SIZE(temp_measure_attr_list, zb_zcl_attr_t), \
          (temp_measure_attr_list),                                 \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        ),                                                          \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT,                \
          ZB_ZCL_ARRAY_SIZE(iluminance_measure_attr_list, zb_zcl_attr_t), \
          (iluminance_measure_attr_list),                           \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        ),                                                          \
        ZB_ZCL_CLUSTER_DESC(                                        \
          ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,               \
          ZB_ZCL_ARRAY_SIZE(humi_meassure_attr_list, zb_zcl_attr_t),\
          (humi_meassure_attr_list),                                \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                               \
          ZB_ZCL_MANUF_CODE_INVALID                                 \
        )                                                           \
      }

/** @brief Declares simple descriptor for the "Device_name" device.
 *  
 *  @param ep_name          Endpoint variable name.
 *  @param ep_id            Endpoint ID.
 *  @param in_clust_num     Number of the supported input clusters.
 *  @param out_clust_num    Number of the supported output clusters.
 */
#define ZB_ZCL_DECLARE_PLANT_GUARD_AIR_EP_DESC(ep_name, ep_id, in_clust_num, out_clust_num) \
  ZB_DECLARE_SIMPLE_DESC(in_clust_num, out_clust_num);                                \
  ZB_AF_SIMPLE_DESC_TYPE(in_clust_num, out_clust_num) simple_desc_##ep_name =         \
  {                                                                                   \
    ep_id,                                                                            \
    ZB_AF_HA_PROFILE_ID,                                                              \
    ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID,                                               \
    ZB_DEVICE_VER_PLANT_GUARD,                                                        \
    0,                                                                                \
    in_clust_num,                                                                     \
    out_clust_num,                                                                    \
    {                                                                                 \
      ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,                                             \
      ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT,                                      \
      ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,                                     \
    }                                                                                 \
  }

#define ZB_PLANT_GUARD_AIR_REPORT_ATTR_COUNT  3                                    /**< Number of attributes mandatory for reporting in the Temperature and Pressure Measurement cluster. */
#define ZB_PLANT_GUARD_AIR_IN_CLUSTER_NUM     3                                 /**< Number of the input (server) clusters in the multisensor device. */
#define ZB_PLANT_GUARD_AIR_OUT_CLUSTER_NUM    0                                    /**< Number of the output (client) clusters in the multisensor device. */


/** @brief Declares endpoint for the plantguard GROUND endpoint.
 *   
 *  @param ep_name          Endpoint variable name.
 *  @param ep_id            Endpoint ID.
 *  @param cluster_list     Endpoint cluster list.
 */
#define ZB_ZCL_DECLARE_PLANT_GUARD_AIR_EP(ep_name, ep_id, cluster_list)              \
  ZB_ZCL_DECLARE_PLANT_GUARD_AIR_EP_DESC(ep_name,                                       \
      ep_id,                                                                      \
      ZB_PLANT_GUARD_AIR_IN_CLUSTER_NUM,                                             \
      ZB_PLANT_GUARD_AIR_OUT_CLUSTER_NUM);                                           \
  ZBOSS_DEVICE_DECLARE_REPORTING_CTX(reporting_info## ep_name,            \
                                     ZB_PLANT_GUARD_AIR_REPORT_ATTR_COUNT);          \
  ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id,                                     \
      ZB_AF_HA_PROFILE_ID,                                                        \
      0,                                                                          \
      NULL,                                                                       \
      ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t),                     \
      cluster_list,                                                               \
      (zb_af_simple_desc_1_1_t*)&simple_desc_##ep_name,                           \
      ZB_PLANT_GUARD_AIR_REPORT_ATTR_COUNT, reporting_info## ep_name, 0, NULL)


ZB_ZCL_DECLARE_TEMP_MEASUREMENT_ATTRIB_LIST(temperature_attr_list, 
                                            &m_dev_ctx.temp_attr.measure_value,
                                            &m_dev_ctx.temp_attr.min_measure_value, 
                                            &m_dev_ctx.temp_attr.max_measure_value, 
                                            &m_dev_ctx.temp_attr.tolerance);

ZB_ZCL_DECLARE_ILLUMINANCE_MEASUREMENT_ATTRIB_LIST(iluminance_attr_list, 
                                            &m_dev_ctx.iluminance_attr.measure_value, 
                                            &m_dev_ctx.iluminance_attr.min_measure_value, 
                                            &m_dev_ctx.iluminance_attr.max_measure_value);

ZB_ZCL_DECLARE_REL_HUMIDITY_MEASUREMENT_ATTRIB_LIST(humidity_attr_list,
                                            &m_dev_ctx.humidity_attr.measure_value,
                                            &m_dev_ctx.humidity_attr.min_measure_value,
                                            &m_dev_ctx.humidity_attr.max_measure_value);

ZB_DECLARE_PLANT_GUARD_AIR_CLUSTER_LIST(plant_guard_air_clusters,
                                     temperature_attr_list,
                                     iluminance_attr_list,
                                     humidity_attr_list);

ZB_ZCL_DECLARE_PLANT_GUARD_AIR_EP(plant_guard_air_ep,
                               PLANT_GUARD_AIR_ENDPOINT,
                               plant_guard_air_clusters);




ZBOSS_DECLARE_DEVICE_CTX_2_EP(plant_guard_sensor_ctx, plant_guard_ground_ep, plant_guard_air_ep);


/**@brief Function for initializing all clusters attributes.
 */
static void multi_sensor_clusters_attr_init(void)
{
    /* Basic cluster attributes data */
    m_dev_ctx.basic_attr.zcl_version   = ZB_ZCL_VERSION;
    m_dev_ctx.basic_attr.app_version   = SENSOR_INIT_BASIC_APP_VERSION;
    m_dev_ctx.basic_attr.stack_version = SENSOR_INIT_BASIC_STACK_VERSION;
    m_dev_ctx.basic_attr.hw_version    = SENSOR_INIT_BASIC_HW_VERSION;

    /* Use ZB_ZCL_SET_STRING_VAL to set strings, because the first byte should
     * contain string length without trailing zero.
     *
     * For example "test" string wil be encoded as:
     *   [(0x4), 't', 'e', 's', 't']
     */
    ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.mf_name,
                          SENSOR_INIT_BASIC_MANUF_NAME,
                          ZB_ZCL_STRING_CONST_SIZE(SENSOR_INIT_BASIC_MANUF_NAME));

    ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.model_id,
                          SENSOR_INIT_BASIC_MODEL_ID,
                          ZB_ZCL_STRING_CONST_SIZE(SENSOR_INIT_BASIC_MODEL_ID));

    ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.date_code,
                          SENSOR_INIT_BASIC_DATE_CODE,
                          ZB_ZCL_STRING_CONST_SIZE(SENSOR_INIT_BASIC_DATE_CODE));

    m_dev_ctx.basic_attr.power_source = SENSOR_INIT_BASIC_POWER_SOURCE;

    ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.location_id,
                          SENSOR_INIT_BASIC_LOCATION_DESC,
                          ZB_ZCL_STRING_CONST_SIZE(SENSOR_INIT_BASIC_LOCATION_DESC));


    m_dev_ctx.basic_attr.ph_env = SENSOR_INIT_BASIC_PH_ENV;

    /* Identify cluster attributes data */
    m_dev_ctx.identify_attr.identify_time        = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;

    /* Temperature measurement cluster attributes data */
    m_dev_ctx.temp_attr.measure_value            = ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_UNKNOWN;
    m_dev_ctx.temp_attr.min_measure_value        = ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_MIN_VALUE;
    m_dev_ctx.temp_attr.max_measure_value        = ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_MAX_VALUE;
    m_dev_ctx.temp_attr.tolerance                = ZB_ZCL_ATTR_TEMP_MEASUREMENT_TOLERANCE_MAX_VALUE;

    /* Pressure measurement cluster attributes data */
    m_dev_ctx.iluminance_attr.measure_value            = ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_UNKNOWN;
    m_dev_ctx.iluminance_attr.min_measure_value        = ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MIN_VALUE_MIN_VALUE;
    m_dev_ctx.iluminance_attr.max_measure_value        = ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MAX_VALUE_MAX_VALUE;

    m_dev_ctx.humidity_attr.measure_value        = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_UNKNOWN;
    m_dev_ctx.humidity_attr.min_measure_value    = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_MIN_VALUE;
    m_dev_ctx.humidity_attr.max_measure_value    = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_MAX_VALUE;

    m_dev_ctx.soil_moisture_attr.measure_value        = ZB_ZCL_ATTR_SOIL_MOISTURE_MEASUREMENT_VALUE_UNKNOWN;
    m_dev_ctx.soil_moisture_attr.min_measure_value    = ZB_ZCL_ATTR_SOIL_MOISTURE_MEASUREMENT_MIN_VALUE_MIN_VALUE;
    m_dev_ctx.soil_moisture_attr.max_measure_value    = ZB_ZCL_ATTR_SOIL_MOISTURE_MEASUREMENT_MAX_VALUE_MAX_VALUE;


    m_dev_ctx.power_config_attr.voltage = 29;
    m_dev_ctx.power_config_attr.size = ZB_ZCL_POWER_CONFIG_BATTERY_SIZE_AAA;
    m_dev_ctx.power_config_attr.quantity= 1;
    m_dev_ctx.power_config_attr.rated_voltage = 30;
    m_dev_ctx.power_config_attr.alarm_mask = ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_DEFAULT_VALUE;
    m_dev_ctx.power_config_attr.voltage_min_threshold = 20;
    m_dev_ctx.power_config_attr.battery_percentage_remaining = 255;
}





/**@brief Function for handling nrf app timer.
 * 
 * @param[IN]   context   Void pointer to context function is called with.
 * 
 * @details Function is called with pointer to sensor_device_ep_ctx_t as argument.
 */
void zigbee_app_update(measurments_t measurements)
{
    LOG_INF("Updating zigbee measurements attribiutes f");

    zb_zcl_status_t zcl_status;

    /* Get new temperature measured value */
    zb_int16_t new_temp_value = (zb_int16_t)(measurements.temperature_ground * 100);
    //LOG_INF("bme_data.temperature: %d, %x" , new_temp_value, new_temp_value);
    zcl_status = zb_zcl_set_attr_val(PLANT_GUARD_GROUND_ENDPOINT, 
                                     ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, 
                                     ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                     ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, 
                                     (zb_uint8_t *)&new_temp_value, 
                                     ZB_FALSE);
    if(zcl_status != ZB_ZCL_STATUS_SUCCESS)
    {
        LOG_INF("Set temperature value fail. zcl_status: %d", zcl_status);
    }

    zb_int16_t new_soil_moisture_value =  (zb_int16_t)(measurements.soil_moisture*100);
    //sLOG_INF("bme_data.humidity/10: %d, %x" , new_hum_value, new_hum_value);
    zcl_status = zb_zcl_set_attr_val(PLANT_GUARD_GROUND_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_SOIL_MOISTURE_MEASUREMENT, 
                                     ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                     ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, 
                                     (zb_uint8_t *)&new_soil_moisture_value, 
                                     ZB_FALSE);
    if(zcl_status != ZB_ZCL_STATUS_SUCCESS)
    {
        LOG_INF("Set humidity value fail. zcl_status: %d", zcl_status);
    }

   // LOG_INF("bme_data.pressure: %d, %x" , bme_data.pressure, bme_data.pressure);
    /* Get new pressure measured value */
    zb_int16_t new_iluminance_value = (zb_int16_t)measurements.lux;
    //LOG_INF("bme_data.pressure/100: %d, %x" , new_pres_value, new_pres_value);
    zcl_status = zb_zcl_set_attr_val(PLANT_GUARD_AIR_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT, 
                                     ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                     ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_ID, 
                                     (zb_uint8_t *)&new_iluminance_value, 
                                     ZB_FALSE);
    if(zcl_status != ZB_ZCL_STATUS_SUCCESS)
    {
        LOG_INF("Set iluminance value fail. zcl_status: %d", zcl_status);
    }

    zb_int16_t new_hum_value =  (zb_int16_t)(measurements.air_humidity*100);
    //sLOG_INF("bme_data.humidity/10: %d, %x" , new_hum_value, new_hum_value);
    zcl_status = zb_zcl_set_attr_val(PLANT_GUARD_AIR_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, 
                                     ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                     ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, 
                                     (zb_uint8_t *)&new_hum_value, 
                                     ZB_FALSE);
    if(zcl_status != ZB_ZCL_STATUS_SUCCESS)
    {
        LOG_INF("Set humidity value fail. zcl_status: %d", zcl_status);
    }

    zb_int16_t new_air_temp =  (zb_int16_t)(measurements.air_temperature*100);
    //sLOG_INF("bme_data.humidity/10: %d, %x" , new_hum_value, new_hum_value);
    zcl_status = zb_zcl_set_attr_val(PLANT_GUARD_AIR_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, 
                                     ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                     ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, 
                                     (zb_uint8_t *)&new_air_temp, 
                                     ZB_FALSE);
    if(zcl_status != ZB_ZCL_STATUS_SUCCESS)
    {
        LOG_INF("Set air temperature value fail. zcl_status: %d", zcl_status);
    }

    zb_uint8_t percent =  (zb_int_t)(255 * measurements.battery/100);
    //sLOG_INF("bme_data.humidity/10: %d, %x" , new_hum_value, new_hum_value);
    zcl_status = zb_zcl_set_attr_val(PLANT_GUARD_GROUND_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_POWER_CONFIG, 
                                     ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                     ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID, 
                                     &percent, 
                                     ZB_FALSE);
    if(zcl_status != ZB_ZCL_STATUS_SUCCESS)
    {
        LOG_INF("Set battert percetage value fail. zcl_status: %d", zcl_status);
    }
}


/**@brief Function to handle identify notification events on the first endpoint.
 *
 * @param  bufid  Unused parameter, required by ZBOSS scheduler API.
 */
static void identify_cb(zb_bufid_t bufid)
{
	zb_ret_t zb_err_code;

	if (bufid) {
		/* Schedule a self-scheduling function that will toggle the LED */
		// ZB_SCHEDULE_APP_CALLBACK(toggle_identify_led, bufid);
    LOG_INF("identify started.");
	} else {
		/* Cancel the toggling function alarm and turn off LED */
		// zb_err_code = ZB_SCHEDULE_APP_ALARM_CANCEL(toggle_identify_led, ZB_ALARM_ANY_PARAM);
		ZVUNUSED(zb_err_code);
    LOG_INF("identify end.");
		// dk_set_led(IDENTIFY_LED, 0);
	}
}

static zb_uint8_t zcl_device_cb(zb_bufid_t bufid)
{

    zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(bufid, zb_zcl_parsed_hdr_t);
   
    LOG_INF("**********************************");
    LOG_INF("zcl endpoint: 0x%x", cmd_info->addr_data.common_data.dst_endpoint);
    LOG_INF("zcl cluster id: 0x%x", cmd_info->cluster_id);
    LOG_INF("zcl cmd_id: 0x%x", cmd_info->cmd_id);


    switch (cmd_info->cmd_id)
    {
        case ZB_ZCL_CMD_CONFIG_REPORT:
        {
            LOG_INF("ZB_ZCL_CMD_CONFIG_REPORT");
            zb_zcl_report_attr_req_t *rep_attr_req;
            rep_attr_req = (zb_zcl_report_attr_req_t *) zb_buf_begin(bufid);  // <--- this approach not feaching data from buffer
            // ZB_ZCL_GENERAL_GET_NEXT_REPORT_ATTR_REQ(bufid, rep_attr_req);  // <--- this takes data form buffer and stack have no data to process

            LOG_INF("ZB_ZCL_CMD_CONFIG_REPORT->attr_id:  0x%x", rep_attr_req->attr_id); 
            LOG_INF("ZB_ZCL_CMD_CONFIG_REPORT->attr_type:  0x%x", rep_attr_req->attr_type); 
            LOG_INF("ZB_ZCL_CMD_CONFIG_REPORT->attr_value:  0x%x", rep_attr_req->attr_value[0]); 

        }
        break;
        case ZB_ZCL_CMD_READ_ATTRIB:
        {
            zb_zcl_read_attr_req_t * read_attr;
            read_attr = (zb_zcl_read_attr_req_t *) zb_buf_begin(bufid);
            LOG_INF("ZB_ZCL_CMD_READ_ATTRIB->attr_id:  0x%x", read_attr->attr_id[0]); 
        }
        break;
        default:
          break;
    }

    //NRF_LOG_INFO("zcl_device_cb status: %hd", p_device_cb_param->status);
    
/**
 * ZB_TRUE – If the ZCL command was processed by the application.
 * ZB_FALSE – The ZCL command will be proceeded by the stack.  
 */
  return ZB_FALSE;
}



void zigbee_app_init() {
  /* Register device context (endpoints). */
	ZB_AF_REGISTER_DEVICE_CTX(&plant_guard_sensor_ctx);

	multi_sensor_clusters_attr_init();

	/* Register handlers to identify notifications */
	ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(PLANT_GUARD_GROUND_ENDPOINT, identify_cb);

  ZB_AF_SET_ENDPOINT_HANDLER(PLANT_GUARD_AIR_ENDPOINT, zcl_device_cb);
  ZB_AF_SET_ENDPOINT_HANDLER(PLANT_GUARD_GROUND_ENDPOINT, zcl_device_cb);

  zigbee_configure_sleepy_behavior(true);

  LOG_INF("initalizing zigbee.");
}

void zigbee_app_start() {
  zigbee_enable(); 
  LOG_INF("starting zigbee thread");
}

#define ZB_DEV_REJOIN_TIMEOUT_MS (1000 * 20)
static bool reset_done = false;

void zigbee_app_factory_reset() {
  LOG_DBG("Schedule Factory Reset; stop timer; set factory_reset_done flag");
  if (reset_done == false) {
      ZB_SCHEDULE_APP_CALLBACK(zb_bdb_reset_via_local_action, 0);
      reset_done = true;
  } else {
    user_input_indicate();
  }


}


/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer
 *                      used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid)
{
    zb_zdo_app_signal_hdr_t *sig_hndler = NULL;
    zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &sig_hndler);
    zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
  

    if(sig != 22) {
      LOG_INF("################################");
      LOG_INF("ZBOSS Signal Handler invoked.");
      LOG_INF("Signal type: %d (0x%02X)", sig, sig);
      LOG_INF("Signal status: %d", status);
      LOG_INF("++++++++++++++++++++++++++++++++");
    }

    /* Log specific signals */
    switch (sig) {
        case ZB_ZDO_SIGNAL_DEVICE_ANNCE:
            LOG_INF("Device announcement signal received.");
            break;

        case ZB_ZDO_SIGNAL_LEAVE:
            LOG_INF("Device leave signal received.");
            if (status == RET_OK) {
                LOG_INF("Device left the network successfully.");
            } else {
                LOG_ERR("Error during device leave, status: %d", status);
            }
            break;

        case ZB_BDB_SIGNAL_STEERING:
            LOG_INF("ZB_BDB_SIGNAL_STEERING.");
            if (status == RET_OK) {
              reset_done = false;
            }
            break;

        case ZB_BDB_SIGNAL_STEERING_CANCELLED:
            LOG_INF("ZB_BDB_SIGNAL_STEERING_CANCELLED.");
            break;

        default:
            break;
    }


    /* Handle default signal processing */
    ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

    /* All callbacks should either reuse or free passed buffers.
     * If bufid == 0, the buffer is invalid (not passed).
     */
    if (bufid) {
        zb_buf_free(bufid);
    }
}

#endif /* CONFIG_ZIGBEE */

