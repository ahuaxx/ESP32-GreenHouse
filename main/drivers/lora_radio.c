#include "lora_radio.h"

#include <stdio.h>

#include "esp_log.h"

static const char *TAG = "LORA";

esp_err_t lora_radio_init(void)
{
    ESP_LOGI(TAG, "LoRa TX mode initialized");
    ESP_LOGW(TAG, "No LoRa receiver available: TX is currently simulated/logged only");
    return ESP_OK;
}

esp_err_t lora_radio_send_text(const char *text)
{
    if (text == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "LoRa TX payload: %s", text);
    return ESP_OK;
}

esp_err_t lora_radio_send_sensor_data(const sensor_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    char payload[200];

    snprintf(
        payload,
        sizeof(payload),
        "T=%.2fC,H=%.2f%%,P=%.2fhPa,G=%.2fkOhm,L=%uLux,BME=%d,LIGHT=%d",
        data->temperature_c,
        data->humidity_pct,
        data->pressure_hpa,
        data->gas_kohms,
        data->light_lux,
        data->bme680_ok,
        data->light_ok
    );

    return lora_radio_send_text(payload);
}
