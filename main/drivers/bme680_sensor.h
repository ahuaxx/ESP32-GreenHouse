#ifndef BME680_SENSOR_H
#define BME680_SENSOR_H

#include "esp_err.h"
#include "driver/i2c_master.h"

#include "bme68x.h"
#include "app_types.h"

typedef struct
{
    i2c_master_dev_handle_t dev_handle;
    struct bme68x_dev dev;
    struct bme68x_conf conf;
} bme680_sensor_t;

esp_err_t bme680_sensor_init(
    bme680_sensor_t *sensor,
    i2c_master_bus_handle_t bus_handle);

esp_err_t bme680_sensor_read(
    bme680_sensor_t *sensor,
    bme680_measurement_t *measurement);

#endif