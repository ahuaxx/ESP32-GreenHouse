#ifndef TMG39931_SENSOR_H
#define TMG39931_SENSOR_H

#include "esp_err.h"
#include "driver/i2c_master.h"

#include "app_types.h"

typedef struct
{
    i2c_master_dev_handle_t dev_handle;
} tmg39931_sensor_t;

esp_err_t tmg39931_sensor_init(
    tmg39931_sensor_t *sensor,
    i2c_master_bus_handle_t bus_handle);

esp_err_t tmg39931_sensor_read(
    tmg39931_sensor_t *sensor,
    tmg39931_measurement_t *measurement);

#endif