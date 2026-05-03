#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "esp_err.h"
#include "sensor_data.h"

esp_err_t oled_display_init(void);
esp_err_t oled_display_clear(void);
esp_err_t oled_display_draw_text(int x, int y, const char *text);
esp_err_t oled_display_update(void);

esp_err_t oled_display_show_text(const char *line1, const char *line2, const char *line3);
esp_err_t oled_display_show_status(const char *title, const char *status, const char *detail);
esp_err_t oled_display_show_measurements(const sensor_data_t *data);

#endif