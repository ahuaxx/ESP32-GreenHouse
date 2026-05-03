#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

esp_err_t display_task_start(
    QueueHandle_t display_queue,
    EventGroupHandle_t event_group
);

#endif
