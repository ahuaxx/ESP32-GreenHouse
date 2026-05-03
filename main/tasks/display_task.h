#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t display_task_start(QueueHandle_t display_queue);

#ifdef __cplusplus
}
#endif

#endif