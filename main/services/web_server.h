#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_err.h"
#include "sensor_data.h"

esp_err_t web_server_start(void);
esp_err_t web_server_update_data(const sensor_data_t *data);

#endif
