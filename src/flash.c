#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#include "flash.h"
#include "error.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(flash, LOG_LEVEL_DBG);

static struct nvs_fs fs;

#define NVS_PARTITION		storage_partition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)


#define SOIL_DATA_ID 1
#define NOTIFICATION_TIME_DATA_ID 2


int flash_init() {
	int rc = 0;
    struct flash_pages_info info;

	/* define the nvs file system by settings with:
	 *	sector_size equal to the pagesize,
	 *	3 sectors
	 *	starting at NVS_PARTITION_OFFSET
	 */
	fs.flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs.flash_device)) {
		LOG_ERR("Flash device %s is not ready\n", fs.flash_device->name);
		return ERROR_FLASH_INIT;
	}
	fs.offset = NVS_PARTITION_OFFSET;
	rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	if (rc) {
		LOG_ERR("Unable to get page info\n");
		return ERROR_FLASH_INIT;
	}
	fs.sector_size = info.size;
	fs.sector_count = 2U;

	rc = nvs_mount(&fs);
	if (rc) {
		LOG_ERR("Flash Init failed\n");
		return ERROR_FLASH_INIT;
	}

    return ERROR_OK;
}

int flash_write_data(soil_calibration_t data) {
    LOG_INF("writing soil calibration data form flash: [min: %d, max: %d].", data.soil_adc_min, data.soil_adc_max);
	(void)nvs_write(&fs, SOIL_DATA_ID, &data, sizeof(data));
}

int flash_read_data(soil_calibration_t* data) {
    LOG_INF("reading soil calibration data form flash.");
    
    int rc = 0;

    rc = nvs_read(&fs, SOIL_DATA_ID, data, sizeof(soil_calibration_t));
	if (rc > 0) {
		LOG_INF("Id: %d, data: %s\n", SOIL_DATA_ID, data);
        LOG_INF("data readed: [min: %d, max: %d].", data->soil_adc_min, data->soil_adc_max);
        return ERROR_OK;
	} else   {
        soil_calibration_t default_value = {
            .soil_adc_max = 2000,
            .soil_adc_min = 1000
        };

        LOG_WRN("data not founded in flash, writing defaults: [min: %d, max: %d].", default_value.soil_adc_min, default_value.soil_adc_max);

		flash_write_data(default_value);
        
        return ERROR_OK;
	}
}


int flash_write_notification_time(uint16_t data) {
    LOG_INF("writing writing notification time to flash: [time = %d].", data);
	(void)nvs_write(&fs, NOTIFICATION_TIME_DATA_ID, &data, sizeof(uint16_t));
}

int flash_read_notification_time(uint16_t* data) {
    LOG_INF("reading notification time form flash.");
    
    int rc = 0;

    rc = nvs_read(&fs, NOTIFICATION_TIME_DATA_ID, data, sizeof(uint16_t));
	if (rc > 0) {
		LOG_INF("Id: %d, data: %s\n", NOTIFICATION_TIME_DATA_ID, data);
        LOG_INF("data readed: [time = %d].", *data);
        return ERROR_OK;
	} else   {
        uint16_t default_value = 1;

        LOG_WRN("data not founded in flash, writing defaults: [time = %d].", data);

        flash_write_notification_time(default_value);

        return ERROR_OK;
	}
}