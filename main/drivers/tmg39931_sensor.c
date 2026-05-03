#include "tmg39931_sensor.h"
#include "i2c_bus.h"
#include "app_config.h"

#include <string.h>

#include "esp_log.h"
#include "esp_check.h"

#define TMG39931_ID_REG 0x92
#define TMG39931_ENABLE_REG 0x80
#define TMG39931_ATIME_REG 0x81
#define TMG39931_STATUS_REG 0x93
#define TMG39931_CDATAL_REG 0x94

#define TMG39931_ENABLE_PON_AEN 0x03
#define TMG39931_STATUS_AVALID 0x01

static const char *TAG = "TMG39931";

esp_err_t tmg39931_sensor_init(
    tmg39931_sensor_t *sensor,
    i2c_master_bus_handle_t bus_handle)
{
    if (sensor == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    memset(sensor, 0, sizeof(*sensor));

    ESP_RETURN_ON_ERROR(
        app_i2c_add_device(bus_handle, TMG39931_ADDR, &sensor->dev_handle),
        TAG,
        "Failed to add TMG39931 on I2C bus");

    uint8_t id = 0;
    ESP_RETURN_ON_ERROR(
        app_i2c_read_reg(sensor->dev_handle, TMG39931_ID_REG, &id, 1),
        TAG,
        "Failed to read ID register");

    ESP_LOGI(TAG, "ID register: 0x%02X", id);

    ESP_RETURN_ON_ERROR(
        app_i2c_write_reg(sensor->dev_handle, TMG39931_ATIME_REG, 0xDB),
        TAG,
        "Failed to configure ATIME");

    ESP_RETURN_ON_ERROR(
        app_i2c_write_reg(sensor->dev_handle, TMG39931_ENABLE_REG, TMG39931_ENABLE_PON_AEN),
        TAG,
        "Failed to enable ALS");

    ESP_LOGI(TAG, "ALS enabled");

    return ESP_OK;
}

esp_err_t tmg39931_sensor_read(
    tmg39931_sensor_t *sensor,
    tmg39931_measurement_t *measurement)
{
    if (sensor == NULL || measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t status = 0;

    ESP_RETURN_ON_ERROR(
        app_i2c_read_reg(sensor->dev_handle, TMG39931_STATUS_REG, &status, 1),
        TAG,
        "Failed to read status register");

    if ((status & TMG39931_STATUS_AVALID) == 0)
    {
        ESP_LOGW(TAG, "ALS data not ready");
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t raw[8];

    ESP_RETURN_ON_ERROR(
        app_i2c_read_reg(sensor->dev_handle, TMG39931_CDATAL_REG, raw, sizeof(raw)),
        TAG,
        "Failed to read RGB data");

    measurement->clear = ((uint16_t)raw[1] << 8) | raw[0];
    measurement->red = ((uint16_t)raw[3] << 8) | raw[2];
    measurement->green = ((uint16_t)raw[5] << 8) | raw[4];
    measurement->blue = ((uint16_t)raw[7] << 8) | raw[6];

    return ESP_OK;
}