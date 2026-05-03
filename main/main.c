#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "driver/i2c_master.h"

#include "i2c_bus.h"
#include "sensor_data.h"
#include "sensor_service.h"
#include "sensor_task.h"
#include "display_task.h"
#include "lora_tx_task.h"

static const char *TAG = "APP";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting greenhouse probe");

    i2c_master_bus_handle_t i2c_bus_handle = NULL;

    ESP_ERROR_CHECK(app_i2c_bus_init(&i2c_bus_handle));
    ESP_ERROR_CHECK(sensor_service_init());

    QueueHandle_t display_queue = xQueueCreate(1, sizeof(sensor_data_t));
    QueueHandle_t lora_queue = xQueueCreate(1, sizeof(sensor_data_t));

    if (display_queue == NULL || lora_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queues");
        return;
    }

    ESP_ERROR_CHECK(sensor_task_start(i2c_bus_handle, display_queue, lora_queue));
    ESP_ERROR_CHECK(display_task_start(display_queue));
    ESP_ERROR_CHECK(lora_tx_task_start(lora_queue));

    ESP_LOGI(TAG, "All tasks started");
}