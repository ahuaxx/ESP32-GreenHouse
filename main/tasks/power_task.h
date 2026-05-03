#ifndef POWER_TASK_H
#define POWER_TASK_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define APP_EVENT_DISPLAY_DONE BIT0
#define APP_EVENT_LORA_DONE    BIT1

esp_err_t power_task_start(EventGroupHandle_t event_group);

#endif // POWER_TASK_H