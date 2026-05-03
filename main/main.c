#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/i2c_master.h"

#include "i2c_bus.h"
#include "sensor_data.h"
#include "sensor_service.h"
#include "sensor_task.h"
#include "display_task.h"
#include "lora_tx_task.h"
#include "power_task.h"
#include "power_service.h"
#include "wifi.h"
#include "web_server.h"

static const char *TAG = "APP";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting greenhouse probe");

    ESP_ERROR_CHECK(power_service_log_wakeup_reason());

    i2c_master_bus_handle_t i2c_bus_handle = NULL;

    ESP_ERROR_CHECK(app_i2c_bus_init(&i2c_bus_handle));
    ESP_ERROR_CHECK(sensor_service_init());

    QueueHandle_t display_queue = xQueueCreate(1, sizeof(sensor_data_t));
    QueueHandle_t lora_queue = xQueueCreate(1, sizeof(sensor_data_t));
    EventGroupHandle_t app_event_group = xEventGroupCreate();

    if (display_queue == NULL || lora_queue == NULL || app_event_group == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queues/event group");
        return;
    }

    ESP_ERROR_CHECK(wifi_init_sta());
    esp_err_t wifi_ret = wifi_wait_for_connection(10000);
    if (wifi_ret != ESP_OK)
    {
        ESP_LOGW(TAG, "WiFi connection timeout, web server will still start");
    }
    ESP_ERROR_CHECK(web_server_start());

    ESP_ERROR_CHECK(sensor_task_start(i2c_bus_handle, display_queue, lora_queue));

    ESP_ERROR_CHECK(display_task_start(display_queue, app_event_group));
    ESP_ERROR_CHECK(lora_tx_task_start(lora_queue, app_event_group));

    ESP_ERROR_CHECK(power_task_start(app_event_group));

    ESP_LOGI(TAG, "All tasks started");
}
