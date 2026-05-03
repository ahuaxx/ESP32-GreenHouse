#include "lora_tx_task.h"

#include <stdio.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lora_radio.h"
#include "sensor_data.h"

static const char *TAG = "LORA_TX_TASK";

static QueueHandle_t s_sensor_data_queue = NULL;

static void lora_tx_task(void *arg)
{
    (void)arg;

    ESP_ERROR_CHECK(lora_radio_init());

    sensor_data_t data;
    char payload[128];

    while (1)
    {
        if (xQueueReceive(s_sensor_data_queue, &data, portMAX_DELAY) == pdTRUE)
        {

            snprintf(payload, sizeof(payload),
                     "T=%.1f;H=%.1f;P=%.1f;G=%.1f;L=%u;BME=%d;LIGHT=%d",
                     data.temperature_c,
                     data.humidity_pct,
                     data.pressure_hpa,
                     data.gas_kohms,
                     data.light_lux,
                     data.bme680_ok,
                     data.light_ok);

            lora_radio_send_text(payload);
        }
    }
}

esp_err_t lora_tx_task_start(QueueHandle_t sensor_data_queue)
{
    if (sensor_data_queue == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    s_sensor_data_queue = sensor_data_queue;

    BaseType_t ret = xTaskCreate(
        lora_tx_task,
        "lora_tx_task",
        4096,
        NULL,
        5,
        NULL);

    if (ret != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create LoRa TX task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "LoRa TX task started");
    return ESP_OK;
}