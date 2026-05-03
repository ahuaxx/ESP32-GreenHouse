#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/i2c_master.h"

esp_err_t sensor_task_start(
    i2c_master_bus_handle_t i2c_bus_handle,
    QueueHandle_t display_queue,
    QueueHandle_t lora_queue
);

#endif // SENSOR_TASK_H
