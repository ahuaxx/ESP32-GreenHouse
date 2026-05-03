#ifndef POWER_SERVICE_H
#define POWER_SERVICE_H

#include <stdint.h>
#include "esp_err.h"

esp_err_t power_service_log_wakeup_reason(void);
void power_service_enter_deep_sleep(uint32_t sleep_time_sec);

#endif // POWER_SERVICE_H