#include "i2c_bus.h"
#include "app_config.h"

#include "esp_log.h"

static const char *TAG = "I2C_BUS";

esp_err_t app_i2c_bus_init(i2c_master_bus_handle_t *bus_handle)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    return i2c_new_master_bus(&bus_config, bus_handle);
}

esp_err_t app_i2c_add_device(
    i2c_master_bus_handle_t bus_handle,
    uint8_t device_address,
    i2c_master_dev_handle_t *dev_handle)
{
    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = device_address,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    return i2c_master_bus_add_device(bus_handle, &device_config, dev_handle);
}

esp_err_t app_i2c_read_reg(
    i2c_master_dev_handle_t dev_handle,
    uint8_t reg,
    uint8_t *data,
    size_t len)
{
    return i2c_master_transmit_receive(
        dev_handle,
        &reg,
        1,
        data,
        len,
        1000);
}

esp_err_t app_i2c_write_reg(
    i2c_master_dev_handle_t dev_handle,
    uint8_t reg,
    uint8_t value)
{
    uint8_t buffer[2] = {reg, value};

    return i2c_master_transmit(
        dev_handle,
        buffer,
        sizeof(buffer),
        1000);
}

void app_i2c_scan(i2c_master_bus_handle_t bus_handle)
{
#if ENABLE_I2C_SCAN
    ESP_LOGI(TAG, "Scanning I2C bus...");

    for (uint8_t addr = 1; addr < 127; addr++)
    {
        if (i2c_master_probe(bus_handle, addr, 100) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found I2C device at address: 0x%02X", addr);
        }
    }

    ESP_LOGI(TAG, "I2C scan finished");
#else
    (void)bus_handle;
#endif
}