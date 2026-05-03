#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

#include "app_config.h"
#include "app_types.h"
#include "sensor_data.h"

#include "oled_display.h"
#include "bme680_sensor.h"
#include "tmg39931_sensor.h"

static const char *TAG = "MAIN";

static i2c_master_bus_handle_t sensor_i2c_bus = NULL;

static esp_err_t sensor_i2c_bus_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    return i2c_new_master_bus(&bus_config, &sensor_i2c_bus);
}

static void i2c_scan(i2c_master_bus_handle_t bus_handle)
{
    ESP_LOGI(TAG, "Scanning I2C bus...");

    for (uint8_t addr = 1; addr < 127; addr++)
    {
        esp_err_t ret = i2c_master_probe(bus_handle, addr, 100);

        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Found I2C device at 0x%02X", addr);
        }
    }

    ESP_LOGI(TAG, "I2C scan done");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting greenhouse monitor");

    ESP_ERROR_CHECK(oled_display_init());

    ESP_ERROR_CHECK(oled_display_show_status(
        "Serre IoT",
        "Init I2C",
        "..."));

    ESP_ERROR_CHECK(sensor_i2c_bus_init());

    i2c_scan(sensor_i2c_bus);

    bme680_sensor_t bme680;
    tmg39931_sensor_t tmg39931;

    memset(&bme680, 0, sizeof(bme680));
    memset(&tmg39931, 0, sizeof(tmg39931));

    esp_err_t bme_init_err = bme680_sensor_init(&bme680, sensor_i2c_bus);
    esp_err_t light_init_err = tmg39931_sensor_init(&tmg39931, sensor_i2c_bus);

    vTaskDelay(pdMS_TO_TICKS(500));

    sensor_data_t data = {
        .temperature_c = 0.0f,
        .humidity_pct = 0.0f,
        .pressure_hpa = 0.0f,
        .gas_kohms = 0.0f,
        .light_lux = 0,
        .bme680_ok = bme_init_err == ESP_OK,
        .light_ok = light_init_err == ESP_OK,
    };

    if (data.bme680_ok && data.light_ok)
    {
        ESP_ERROR_CHECK(oled_display_show_status(
            "Serre IoT",
            "Capteurs OK",
            "Lecture..."));
    }
    else
    {
        ESP_LOGW(TAG, "BME680 init: %s", esp_err_to_name(bme_init_err));
        ESP_LOGW(TAG, "TMG39931 init: %s", esp_err_to_name(light_init_err));

        ESP_ERROR_CHECK(oled_display_show_measurements(&data));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    while (1)
    {
        bme680_measurement_t bme_measurement;
        tmg39931_measurement_t light_measurement;

        memset(&bme_measurement, 0, sizeof(bme_measurement));
        memset(&light_measurement, 0, sizeof(light_measurement));
        
        esp_err_t bme_read_err = ESP_FAIL;
        esp_err_t light_read_err = ESP_FAIL;

        if (data.bme680_ok)
        {
            bme_read_err = bme680_sensor_read(&bme680, &bme_measurement);
        }

        if (data.light_ok)
        {
            light_read_err = tmg39931_sensor_read(&tmg39931, &light_measurement);
        }

        data.bme680_ok = bme_read_err == ESP_OK;
        data.light_ok = light_read_err == ESP_OK;

        if (data.bme680_ok)
        {
            data.temperature_c = bme_measurement.temperature;
            data.humidity_pct = bme_measurement.humidity;
            data.pressure_hpa = bme_measurement.pressure_hpa;

            // conversion Ohms → kOhms
            data.gas_kohms = bme_measurement.gas_resistance / 1000.0f;
        }

        if (data.light_ok)
        {
            // approximation de luminosité
            data.light_lux = light_measurement.clear;
        }

        ESP_ERROR_CHECK(oled_display_show_measurements(&data));

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_REFRESH_PERIOD_MS));
    }
}