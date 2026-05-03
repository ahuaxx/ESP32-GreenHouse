#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include "esp_err.h"
#include "sensor_data.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t lora_radio_init(void);
esp_err_t lora_radio_send_text(const char *text);
esp_err_t lora_radio_send_sensor_data(const sensor_data_t *data);

#ifdef __cplusplus
}
#endif

#endif // LORA_RADIO_H
