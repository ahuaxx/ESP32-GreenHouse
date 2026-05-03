#include "web_server.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "WEB";

static sensor_data_t g_data = {0};
static SemaphoreHandle_t g_data_mutex = NULL;

static esp_err_t root_get_handler(httpd_req_t *req)
{
    char html[1024];
    sensor_data_t data = {0};

    if (g_data_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(g_data_mutex, portMAX_DELAY);
    data = g_data;
    xSemaphoreGive(g_data_mutex);

    const char *bme_status = data.bme680_ok ? "OK" : "FAIL";
    const char *light_status = data.light_ok ? "OK" : "FAIL";

    snprintf(html, sizeof(html),
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<meta http-equiv='refresh' content='2'>"
        "<title>Greenhouse Monitor</title>"

        "<style>"
        "body{font-family:Arial;background:#0f172a;color:#e2e8f0;text-align:center;margin:0;padding:20px;}"
        ".card{background:#1e293b;border-radius:12px;padding:20px;margin:10px auto;width:300px;"
        "box-shadow:0 4px 10px rgba(0,0,0,0.3);}"
        ".ok{color:#22c55e;}"
        ".fail{color:#ef4444;}"
        "h1{margin-bottom:20px;}"
        "</style>"
        "</head>"

        "<body>"
        "<h1>🌱 Greenhouse Monitor</h1>"

        "<div class='card'>"
        "<h2>Temperature</h2>"
        "<p>%.2f °C</p>"
        "</div>"

        "<div class='card'>"
        "<h2>Humidity</h2>"
        "<p>%.2f %%</p>"
        "</div>"

        "<div class='card'>"
        "<h2>Pressure</h2>"
        "<p>%.2f hPa</p>"
        "</div>"

        "<div class='card'>"
        "<h2>Light</h2>"
        "<p>%u lux</p>"
        "</div>"

        "<div class='card'>"
        "<h2>Status</h2>"
        "<p>BME680: <span class='%s'>%s</span></p>"
        "<p>Light: <span class='%s'>%s</span></p>"
        "</div>"

        "</body>"
        "</html>",

        data.temperature_c,
        data.humidity_pct,
        data.pressure_hpa,
        data.light_lux,
        data.bme680_ok ? "ok" : "fail",
        bme_status,
        data.light_ok ? "ok" : "fail",
        light_status
    );

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t web_server_update_data(const sensor_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (g_data_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(g_data_mutex, portMAX_DELAY);
    g_data = *data;
    xSemaphoreGive(g_data_mutex);
    return ESP_OK;
}

esp_err_t web_server_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (g_data_mutex == NULL) {
        g_data_mutex = xSemaphoreCreateMutex();
        if (g_data_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create web data mutex");
            return ESP_FAIL;
        }
    }

    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server");
        return ESP_FAIL;
    }

    httpd_uri_t uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler
    };

    httpd_register_uri_handler(server, &uri);

    ESP_LOGI(TAG, "Web server started");
    return ESP_OK;
}
