#include "sensor_task.h"

#include "app_config.h"
#include "app_types.h"

#include "bme680_sensor.h"
#include "tmg39931_sensor.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSOR_TASK";

typedef struct
{
    i2c_master_bus_handle_t bus_handle;
} sensor_task_context_t;

static sensor_task_context_t sensor_task_ctx;

static void sensor_task(void *arg)
{
    sensor_task_context_t *ctx = (sensor_task_context_t *)arg;

    bme680_sensor_t bme680;
    tmg39931_sensor_t light_sensor;

    ESP_ERROR_CHECK(bme680_sensor_init(&bme680, ctx->bus_handle));
    ESP_ERROR_CHECK(tmg39931_sensor_init(&light_sensor, ctx->bus_handle));

    vTaskDelay(pdMS_TO_TICKS(500));

    while (1)
    {
        bme680_measurement_t bme_data;
        tmg39931_measurement_t light_data;

        if (bme680_sensor_read(&bme680, &bme_data) == ESP_OK)
        {
            ESP_LOGI("BME680", "Temperature: %.2f °C", bme_data.temperature);
            ESP_LOGI("BME680", "Humidity: %.2f %%", bme_data.humidity);
            ESP_LOGI("BME680", "Pressure: %.2f hPa", bme_data.pressure_hpa);
            ESP_LOGI("BME680", "Gas resistance: %.2f Ohm", bme_data.gas_resistance);
        }

        if (tmg39931_sensor_read(&light_sensor, &light_data) == ESP_OK)
        {
            ESP_LOGI(
                "TMG39931",
                "Clear: %u | R: %u | G: %u | B: %u",
                light_data.clear,
                light_data.red,
                light_data.green,
                light_data.blue);
        }

        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_PERIOD_MS));
    }
}

void sensor_task_start(i2c_master_bus_handle_t bus_handle)
{
    sensor_task_ctx.bus_handle = bus_handle;

    ESP_LOGI(TAG, "Starting sensor task");

    xTaskCreate(
        sensor_task,
        "sensor_task",
        SENSOR_TASK_STACK_SIZE,
        &sensor_task_ctx,
        SENSOR_TASK_PRIORITY,
        NULL);
}