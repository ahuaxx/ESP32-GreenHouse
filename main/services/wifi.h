#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>

#include "esp_err.h"

esp_err_t wifi_init_sta(void);
esp_err_t wifi_wait_for_connection(uint32_t timeout_ms);

#endif
