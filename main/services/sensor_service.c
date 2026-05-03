#include "sensor_service.h"

#include <string.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "SENSOR_SERVICE";

typedef struct
{
    sensor_measurement_t latest_measurement;

    bool has_bme680_data;
    bool has_light_data;

    SemaphoreHandle_t mutex;
} sensor_service_context_t;

static sensor_service_context_t ctx = {0};

esp_err_t sensor_service_init(void)
{
    memset(&ctx.latest_measurement, 0, sizeof(ctx.latest_measurement));

    ctx.has_bme680_data = false;
    ctx.has_light_data = false;

    ctx.mutex = xSemaphoreCreateMutex();

    if (ctx.mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sensor service initialized");

    return ESP_OK;
}

esp_err_t sensor_service_update_bme680(const bme680_measurement_t *measurement)
{
    if (measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (ctx.mutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(ctx.mutex, portMAX_DELAY);

    ctx.latest_measurement.bme680 = *measurement;
    ctx.has_bme680_data = true;

    xSemaphoreGive(ctx.mutex);

    return ESP_OK;
}

esp_err_t sensor_service_update_light(const tmg39931_measurement_t *measurement)
{
    if (measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (ctx.mutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(ctx.mutex, portMAX_DELAY);

    ctx.latest_measurement.light = *measurement;
    ctx.has_light_data = true;

    xSemaphoreGive(ctx.mutex);

    return ESP_OK;
}

esp_err_t sensor_service_get_latest(sensor_measurement_t *measurement)
{
    if (measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (ctx.mutex == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(ctx.mutex, portMAX_DELAY);

    *measurement = ctx.latest_measurement;

    xSemaphoreGive(ctx.mutex);

    return ESP_OK;
}

bool sensor_service_has_bme680_data(void)
{
    if (ctx.mutex == NULL)
    {
        return false;
    }

    xSemaphoreTake(ctx.mutex, portMAX_DELAY);

    bool result = ctx.has_bme680_data;

    xSemaphoreGive(ctx.mutex);

    return result;
}

bool sensor_service_has_light_data(void)
{
    if (ctx.mutex == NULL)
    {
        return false;
    }

    xSemaphoreTake(ctx.mutex, portMAX_DELAY);

    bool result = ctx.has_light_data;

    xSemaphoreGive(ctx.mutex);

    return result;
}