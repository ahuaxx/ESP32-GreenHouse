#include "power_service.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_err.h"

static const char *TAG = "POWER";

esp_err_t power_service_log_wakeup_reason(void)
{
    uint32_t causes = esp_sleep_get_wakeup_causes();
    
    if (causes & BIT(ESP_SLEEP_WAKEUP_TIMER))
    {
        ESP_LOGI(TAG, "Wakeup: timer");
    }
    else if (causes == 0)
    {
        ESP_LOGI(TAG, "Wakeup: fresh boot");
    }
    else
    {
        ESP_LOGI(TAG, "Wakeup: other cause (%lu)", causes);
    }

    return ESP_OK;
}

void power_service_enter_deep_sleep(uint32_t sleep_time_sec)
{
    ESP_LOGI(TAG, "Entering deep sleep for %lu seconds", sleep_time_sec);

    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(
        (uint64_t)sleep_time_sec * 1000000ULL));

    esp_deep_sleep_start();
}