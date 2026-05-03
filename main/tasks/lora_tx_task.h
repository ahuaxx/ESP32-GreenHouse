#ifndef LORA_TX_TASK_H
#define LORA_TX_TASK_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

esp_err_t lora_tx_task_start(
    QueueHandle_t lora_queue,
    EventGroupHandle_t event_group
);

#endif
