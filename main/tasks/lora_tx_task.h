#ifndef LORA_TX_TASK_H
#define LORA_TX_TASK_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "sensor_data.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t lora_tx_task_start(QueueHandle_t sensor_data_queue);

#ifdef __cplusplus
}
#endif

#endif