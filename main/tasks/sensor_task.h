#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "esp_err.h"

#include "driver/i2c_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sensor_task_start(
    i2c_master_bus_handle_t bus_handle,
    QueueHandle_t display_queue,
    QueueHandle_t lora_queue);

#ifdef __cplusplus
}
#endif

#endif