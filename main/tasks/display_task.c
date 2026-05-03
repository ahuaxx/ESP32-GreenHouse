#include "display_task.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sensor_data.h"
#include "oled_display.h"
#include "power_task.h"
#include "web_server.h"

static const char *TAG = "DISPLAY_TASK";

typedef struct
{
    QueueHandle_t queue;
    EventGroupHandle_t event_group;
} display_task_ctx_t;

static display_task_ctx_t s_display_ctx = {0};

static void display_task(void *arg)
{
    display_task_ctx_t *ctx = (display_task_ctx_t *)arg;

    if (ctx == NULL || ctx->queue == NULL || ctx->event_group == NULL)
    {
        ESP_LOGE(TAG, "Invalid display task context");
        vTaskDelete(NULL);
        return;
    }

    sensor_data_t data;

    ESP_LOGI(TAG, "Display task started");

    ESP_ERROR_CHECK(oled_display_init());

    while (1)
    {
        if (xQueueReceive(ctx->queue, &data, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "Sensor data received");

            ESP_ERROR_CHECK(web_server_update_data(&data));

            ESP_ERROR_CHECK(oled_display_show_measurements(&data));

            xEventGroupSetBits(ctx->event_group, APP_EVENT_DISPLAY_DONE);

            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

esp_err_t display_task_start(QueueHandle_t display_queue, EventGroupHandle_t event_group)
{
    if (display_queue == NULL || event_group == NULL)
    {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    s_display_ctx.queue = display_queue;
    s_display_ctx.event_group = event_group;

    BaseType_t ret = xTaskCreate(
        display_task,
        "display_task",
        4096,
        &s_display_ctx,
        5,
        NULL);

    return ret == pdPASS ? ESP_OK : ESP_FAIL;
}
