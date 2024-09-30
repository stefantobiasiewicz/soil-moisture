#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#include "flash.h"
#include "main.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(flash, LOG_LEVEL_DBG);

static struct nvs_fs fs;

#define NVS_PARTITION		storage_partition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)


#define SOIL_DATA_ID 1
#define SLEEP_TIME_DATA_ID 2


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
	fs.sector_count = 4U;

	LOG_INF("[fs.sector_size: %d, fs.sector_count: %d].", info.size, fs.sector_count);

	rc = nvs_mount(&fs);
	if (rc) {
		LOG_ERR("Flash Init failed\n");
		return ERROR_FLASH_INIT;
	}

    return ERROR_OK;
}

void flash_write_calibration_data(soil_calibration_t data) {
    LOG_INF("writing soil calibration data form flash: [min: %d, max: %d].", data.soil_adc_soil_min, data.soil_adc_soil_max);
	nvs_write(&fs, SOIL_DATA_ID, &data, sizeof(data));
}

int flash_read_calibration_data(soil_calibration_t* data) {
    LOG_INF("reading soil calibration data form flash.");
    
    int rc = 0;

    rc = nvs_read(&fs, SOIL_DATA_ID, data, sizeof(soil_calibration_t));
	if (rc > 0) {
		LOG_INF("Id: %d, data: %s\n", SOIL_DATA_ID, (char* )data);
        LOG_INF("data readed: [min: %d, max: %d].", data->soil_adc_soil_min, data->soil_adc_soil_max);
        return ERROR_OK;
	} else   {
		data->soil_adc_soil_max = 2000;
		data->soil_adc_soil_min = 1000;

        LOG_WRN("data not founded in flash, writing defaults: [min: %d, max: %d].", data->soil_adc_soil_max, data->soil_adc_soil_min);

		flash_write_calibration_data(*data);
        
        return ERROR_OK;
	}
}


void flash_write_sleep_time(uint16_t data) {
    LOG_INF("writing writing sleep time to flash: [time = %d].", (int) data);
	nvs_write(&fs, SLEEP_TIME_DATA_ID, &data, sizeof(data));
}

int flash_read_sleep_time(uint16_t* data) {
    LOG_INF("reading sleep time form flash.");
    
    int rc = 0;

    rc = nvs_read(&fs, SLEEP_TIME_DATA_ID, data, sizeof(uint16_t));
	if (rc > 0) {
		LOG_INF("Id: %d, data: %d\n", SLEEP_TIME_DATA_ID, *data);
        LOG_INF("data readed: [time = %d].", *data);
        return ERROR_OK;
	} else   {
        *data = 5;

        LOG_WRN("data not founded in flash, writing defaults: [time = %d].", *data);

        flash_write_sleep_time(*data);

        return ERROR_OK;
	}
}