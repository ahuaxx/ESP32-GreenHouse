#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <stdbool.h>

#include "esp_err.h"
#include "app_types.h"

esp_err_t sensor_service_init(void);

esp_err_t sensor_service_update_bme680(const bme680_measurement_t *measurement);
esp_err_t sensor_service_update_light(const tmg39931_measurement_t *measurement);

esp_err_t sensor_service_get_latest(sensor_measurement_t *measurement);

bool sensor_service_has_bme680_data(void);
bool sensor_service_has_light_data(void);

#endif