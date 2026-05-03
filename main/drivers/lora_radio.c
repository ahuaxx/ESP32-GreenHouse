#include "lora_radio.h"

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