#include "power_task.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"
#include "oled_display.h"
#include "power_service.h"

static const char *TAG = "POWER_TASK";

static void power_task(void *arg)
{
    EventGroupHandle_t event_group = (EventGroupHandle_t)arg;

    if (event_group == NULL)
    {
        ESP_LOGE(TAG, "Invalid event group");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Waiting for display and LoRa completion");

    xEventGroupWaitBits(
        event_group,
        APP_EVENT_DISPLAY_DONE | APP_EVENT_LORA_DONE,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);

    ESP_LOGI(TAG, "Display and LoRa done");

    // ⬇️ Fenêtre serveur web active
    ESP_LOGI(TAG, "Web server active for %d seconds", APP_WEB_ACTIVE_TIME_SEC);

    vTaskDelay(pdMS_TO_TICKS(APP_WEB_ACTIVE_TIME_SEC * 1000));

    ESP_LOGI(TAG, "Entering deep sleep");

    ESP_ERROR_CHECK(oled_display_shutdown());
    power_service_enter_deep_sleep(APP_DEEP_SLEEP_TIME_SEC);

    vTaskDelete(NULL);
}

esp_err_t power_task_start(EventGroupHandle_t event_group)
{
    if (event_group == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    BaseType_t ret = xTaskCreate(
        power_task,
        "power_task",
        4096,
        event_group,
        5,
        NULL);

    return ret == pdPASS ? ESP_OK : ESP_FAIL;
}
