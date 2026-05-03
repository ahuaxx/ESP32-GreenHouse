#include "lora_tx_task.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sensor_data.h"
#include "lora_radio.h"
#include "power_task.h"

static const char *TAG = "LORA_TX_TASK";

typedef struct
{
    QueueHandle_t queue;
    EventGroupHandle_t event_group;
} lora_tx_task_ctx_t;

static lora_tx_task_ctx_t s_lora_ctx = {0};

static void lora_tx_task(void *arg)
{
    lora_tx_task_ctx_t *ctx = (lora_tx_task_ctx_t *)arg;

    if (ctx == NULL || ctx->queue == NULL || ctx->event_group == NULL) {
        ESP_LOGE(TAG, "Invalid LoRa task context");
        vTaskDelete(NULL);
        return;
    }

    sensor_data_t data;

    ESP_LOGI(TAG, "LoRa TX task started");

    ESP_ERROR_CHECK(lora_radio_init());

    if (xQueueReceive(ctx->queue, &data, portMAX_DELAY) == pdTRUE) {
        ESP_LOGI(TAG, "Sensor data received for LoRa TX");

        esp_err_t ret = lora_radio_send_sensor_data(&data);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "LoRa TX failed: %s", esp_err_to_name(ret));
        }

        xEventGroupSetBits(ctx->event_group, APP_EVENT_LORA_DONE);

        ESP_LOGI(TAG, "LoRa TX done");
    }

    vTaskDelete(NULL);
}

esp_err_t lora_tx_task_start(QueueHandle_t lora_queue, EventGroupHandle_t event_group)
{
    if (lora_queue == NULL || event_group == NULL) {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    s_lora_ctx.queue = lora_queue;
    s_lora_ctx.event_group = event_group;

    BaseType_t ret = xTaskCreate(
        lora_tx_task,
        "lora_tx_task",
        4096,
        &s_lora_ctx,
        5,
        NULL
    );

    return ret == pdPASS ? ESP_OK : ESP_FAIL;
}
