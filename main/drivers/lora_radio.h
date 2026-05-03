#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t lora_radio_init(void);
esp_err_t lora_radio_send_text(const char *text);

#ifdef __cplusplus
}
#endif

#endif