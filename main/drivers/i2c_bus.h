#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "esp_err.h"
#include "driver/i2c_master.h"

esp_err_t app_i2c_bus_init(i2c_master_bus_handle_t *bus_handle);
esp_err_t app_i2c_add_device(
    i2c_master_bus_handle_t bus_handle,
    uint8_t device_address,
    i2c_master_dev_handle_t *dev_handle);

esp_err_t app_i2c_read_reg(
    i2c_master_dev_handle_t dev_handle,
    uint8_t reg,
    uint8_t *data,
    size_t len);

esp_err_t app_i2c_write_reg(
    i2c_master_dev_handle_t dev_handle,
    uint8_t reg,
    uint8_t value);

void app_i2c_scan(i2c_master_bus_handle_t bus_handle);

#endif