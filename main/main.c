#include "esp_log.h"
#include "esp_err.h"

#include "i2c_bus.h"
#include "sensor_task.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting greenhouse monitoring node");

    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(app_i2c_bus_init(&bus_handle));

    app_i2c_scan(bus_handle);

    sensor_task_start(bus_handle);

    ESP_LOGI(TAG, "Initialization finished");
}