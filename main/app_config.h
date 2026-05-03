#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "driver/i2c_master.h"

#define ENABLE_I2C_SCAN 1

#define I2C_SDA_GPIO 33
#define I2C_SCL_GPIO 34
#define I2C_PORT I2C_NUM_0
#define I2C_FREQ_HZ 100000

#define BME680_ADDR 0x77
#define TMG39931_ADDR 0x39

#define SENSOR_TASK_STACK_SIZE 4096
#define SENSOR_TASK_PRIORITY 5
#define SENSOR_READ_PERIOD_MS 2000

#endif