#include "bme680_sensor.h"
#include "i2c_bus.h"
#include "app_config.h"

#include <string.h>

#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_check.h"

#include "bme68x_defs.h"

static const char *TAG = "BME680";

static void bme680_delay_us(uint32_t period_us, void *intf_ptr)
{
    (void)intf_ptr;
    esp_rom_delay_us(period_us);
}

static int8_t bme680_i2c_read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr)
{
    bme680_sensor_t *sensor = (bme680_sensor_t *)intf_ptr;

    esp_err_t ret = i2c_master_transmit_receive(
        sensor->dev_handle,
        &reg_addr,
        1,
        reg_data,
        len,
        1000);

    return (ret == ESP_OK) ? BME68X_OK : BME68X_E_COM_FAIL;
}

static int8_t bme680_i2c_write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr)
{
    bme680_sensor_t *sensor = (bme680_sensor_t *)intf_ptr;

    uint8_t buffer[32];

    if (len + 1 > sizeof(buffer))
    {
        return BME68X_E_COM_FAIL;
    }

    buffer[0] = reg_addr;
    memcpy(&buffer[1], reg_data, len);

    esp_err_t ret = i2c_master_transmit(
        sensor->dev_handle,
        buffer,
        len + 1,
        1000);

    return (ret == ESP_OK) ? BME68X_OK : BME68X_E_COM_FAIL;
}

esp_err_t bme680_sensor_init(
    bme680_sensor_t *sensor,
    i2c_master_bus_handle_t bus_handle)
{
    if (sensor == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    memset(sensor, 0, sizeof(*sensor));

    ESP_RETURN_ON_ERROR(
        app_i2c_add_device(bus_handle, BME680_ADDR, &sensor->dev_handle),
        TAG,
        "Failed to add BME680 on I2C bus");

    sensor->dev.intf = BME68X_I2C_INTF;
    sensor->dev.read = bme680_i2c_read;
    sensor->dev.write = bme680_i2c_write;
    sensor->dev.delay_us = bme680_delay_us;
    sensor->dev.intf_ptr = sensor;
    sensor->dev.amb_temp = 25;

    int8_t rslt = bme68x_init(&sensor->dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGE(TAG, "bme68x_init failed: %d", rslt);
        return ESP_FAIL;
    }

    sensor->conf.filter = BME68X_FILTER_OFF;
    sensor->conf.odr = BME68X_ODR_NONE;
    sensor->conf.os_hum = BME68X_OS_2X;
    sensor->conf.os_pres = BME68X_OS_4X;
    sensor->conf.os_temp = BME68X_OS_8X;

    rslt = bme68x_set_conf(&sensor->conf, &sensor->dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGE(TAG, "bme68x_set_conf failed: %d", rslt);
        return ESP_FAIL;
    }

    struct bme68x_heatr_conf heatr_conf = {
        .enable = BME68X_ENABLE,
        .heatr_temp = 300,
        .heatr_dur = 100,
    };

    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &sensor->dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGE(TAG, "bme68x_set_heatr_conf failed: %d", rslt);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "BME680 initialized");

    return ESP_OK;
}

esp_err_t bme680_sensor_read(
    bme680_sensor_t *sensor,
    bme680_measurement_t *measurement)
{
    if (sensor == NULL || measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &sensor->dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGE(TAG, "bme68x_set_op_mode failed: %d", rslt);
        return ESP_FAIL;
    }

    uint32_t delay_us = bme68x_get_meas_dur(
        BME68X_FORCED_MODE,
        &sensor->conf,
        &sensor->dev);

    sensor->dev.delay_us(delay_us, sensor->dev.intf_ptr);

    struct bme68x_data data;
    uint8_t n_fields = 0;

    rslt = bme68x_get_data(
        BME68X_FORCED_MODE,
        &data,
        &n_fields,
        &sensor->dev);

    if (rslt != BME68X_OK || n_fields == 0)
    {
        ESP_LOGW(TAG, "No data: rslt=%d n_fields=%d", rslt, n_fields);
        return ESP_FAIL;
    }

    measurement->temperature = data.temperature;
    measurement->humidity = data.humidity;
    measurement->pressure_hpa = data.pressure / 100.0f;
    measurement->gas_resistance = data.gas_resistance;

    return ESP_OK;
}