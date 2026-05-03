#include "display_task.h"

#include <stdio.h>

#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "oled_display.h"
#include "sensor_data.h"

static const char *TAG = "DISPLAY_TASK";

static QueueHandle_t s_display_queue = NULL;

static void display_task(void *arg)
{
    (void)arg;

    ESP_LOGI(TAG, "Starting display task");

    ESP_ERROR_CHECK(oled_display_init());

    sensor_data_t data;
    char line1[32];
    char line2[32];
    char line3[32];
    char line4[32];

    while (1)
    {
        if (xQueueReceive(s_display_queue, &data, portMAX_DELAY) == pdTRUE)
        {
            snprintf(line1, sizeof(line1), "T: %.1fC H: %.0f%%",
                     data.temperature_c,
                     data.humidity_pct);

            snprintf(line2, sizeof(line2), "P: %.1fhPa",
                     data.pressure_hpa);

            snprintf(line3, sizeof(line3), "Gas: %.1fk",
                     data.gas_kohms);

            snprintf(line4, sizeof(line4), "Light: %u",
                     data.light_lux);

            oled_display_clear();
            oled_display_draw_text(0, 0, line1);
            oled_display_draw_text(0, 16, line2);
            oled_display_draw_text(0, 32, line3);
            oled_display_draw_text(0, 48, line4);
            oled_display_update();

            ESP_LOGI(TAG, "Display updated");
        }
    }
}

esp_err_t display_task_start(QueueHandle_t display_queue)
{
    if (display_queue == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    s_display_queue = display_queue;

    BaseType_t ret = xTaskCreate(
        display_task,
        "display_task",
        4096,
        NULL,
        4,
        NULL);

    if (ret != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create display task");
        return ESP_FAIL;
    }

    return ESP_OK;
}