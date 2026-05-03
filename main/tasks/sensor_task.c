#include "sensor_task.h"

#include "app_config.h"
#include "app_types.h"

#include "bme680_sensor.h"
#include "tmg39931_sensor.h"
#include "sensor_service.h"
#include "sensor_data.h"

#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSOR_TASK";

static QueueHandle_t s_display_queue = NULL;
static QueueHandle_t s_lora_queue = NULL;

typedef struct
{
    i2c_master_bus_handle_t bus_handle;
} sensor_task_context_t;

static sensor_task_context_t sensor_task_ctx;

static void sensor_task(void *arg)
{
    sensor_task_context_t *ctx = (sensor_task_context_t *)arg;

    if (ctx == NULL || ctx->bus_handle == NULL)
    {
        ESP_LOGE(TAG, "Invalid sensor task context");
        vTaskDelete(NULL);
        return;
    }

    bme680_sensor_t bme680;
    tmg39931_sensor_t light_sensor;

    ESP_ERROR_CHECK(bme680_sensor_init(&bme680, ctx->bus_handle));
    ESP_ERROR_CHECK(tmg39931_sensor_init(&light_sensor, ctx->bus_handle));

    vTaskDelay(pdMS_TO_TICKS(500));

    while (1)
    {
        bme680_measurement_t bme_data = {0};
        tmg39931_measurement_t light_data = {0};
        sensor_data_t sensor_data = {0};

        if (bme680_sensor_read(&bme680, &bme_data) == ESP_OK)
        {
            ESP_ERROR_CHECK(sensor_service_update_bme680(&bme_data));

            sensor_data.temperature_c = bme_data.temperature;
            sensor_data.humidity_pct = bme_data.humidity;
            sensor_data.pressure_hpa = bme_data.pressure_hpa;
            sensor_data.gas_kohms = bme_data.gas_resistance / 1000.0f;
            sensor_data.bme680_ok = true;
            sensor_data.light_lux = light_data.clear;
            sensor_data.light_ok = true;

            ESP_LOGI("BME680", "Temperature: %.2f °C", bme_data.temperature);
            ESP_LOGI("BME680", "Humidity: %.2f %%", bme_data.humidity);
            ESP_LOGI("BME680", "Pressure: %.2f hPa", bme_data.pressure_hpa);
            ESP_LOGI("BME680", "Gas resistance: %.2f Ohm", bme_data.gas_resistance);
        }

        if (tmg39931_sensor_read(&light_sensor, &light_data) == ESP_OK)
        {
            ESP_ERROR_CHECK(sensor_service_update_light(&light_data));

            sensor_data.light_lux = light_data.clear;

            ESP_LOGI(
                "TMG39931",
                "Clear: %u | R: %u | G: %u | B: %u",
                light_data.clear,
                light_data.red,
                light_data.green,
                light_data.blue);
        }

        xQueueOverwrite(s_display_queue, &sensor_data);
        xQueueOverwrite(s_lora_queue, &sensor_data);

        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_PERIOD_MS));
    }
}

esp_err_t sensor_task_start(
    i2c_master_bus_handle_t bus_handle,
    QueueHandle_t display_queue,
    QueueHandle_t lora_queue)
{
    if (bus_handle == NULL || display_queue == NULL || lora_queue == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    sensor_task_ctx.bus_handle = bus_handle;
    s_display_queue = display_queue;
    s_lora_queue = lora_queue;

    BaseType_t ret = xTaskCreate(
        sensor_task,
        "sensor_task",
        4096,
        &sensor_task_ctx,
        5,
        NULL);

    return (ret == pdPASS) ? ESP_OK : ESP_FAIL;
}