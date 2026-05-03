#include "display_task.h"

#include "app_config.h"
#include "app_types.h"
#include "sensor_service.h"
#include "oled_display.h"

#include <stdio.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DISPLAY_TASK";

static void display_task(void *arg)
{
    (void)arg;

    ESP_ERROR_CHECK(oled_display_init());

    while (1)
    {
        sensor_measurement_t data;

        if (sensor_service_get_latest(&data) == ESP_OK &&
            sensor_service_has_bme680_data() &&
            sensor_service_has_light_data())
        {
            char line1[32];
            char line2[32];
            char line3[32];
            char line4[32];

            snprintf(line1, sizeof(line1), "Temp: %.1f C", data.bme680.temperature);
            snprintf(line2, sizeof(line2), "Hum : %.1f %%", data.bme680.humidity);
            snprintf(line3, sizeof(line3), "Pres: %.0f hPa", data.bme680.pressure_hpa);
            snprintf(line4, sizeof(line4), "Light: %u", data.light.clear);

            oled_display_clear();
            oled_display_draw_text(0, 0, line1);
            oled_display_draw_text(0, 16, line2);
            oled_display_draw_text(0, 32, line3);
            oled_display_draw_text(0, 48, line4);
            oled_display_update();
        }
        else
        {
            ESP_LOGW(TAG, "No complete sensor data available yet");

            oled_display_clear();
            oled_display_draw_text(0, 0, "Waiting sensors");
            oled_display_update();
        }

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_REFRESH_PERIOD_MS));
    }
}

void display_task_start(void)
{
    ESP_LOGI(TAG, "Starting display task");

    xTaskCreate(
        display_task,
        "display_task",
        DISPLAY_TASK_STACK_SIZE,
        NULL,
        DISPLAY_TASK_PRIORITY,
        NULL);
}