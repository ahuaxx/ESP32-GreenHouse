#include "oled_display.h"
#include "app_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_ssd1306.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "OLED";

static i2c_master_bus_handle_t oled_bus = NULL;
static esp_lcd_panel_io_handle_t oled_io = NULL;
static esp_lcd_panel_handle_t oled_panel = NULL;

static uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];

static const uint8_t font5x7[][5] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00},
    ['%'] = {0x62, 0x64, 0x08, 0x13, 0x23},
    ['.'] = {0x00, 0x60, 0x60, 0x00, 0x00},
    [':'] = {0x00, 0x36, 0x36, 0x00, 0x00},
    ['-'] = {0x08, 0x08, 0x08, 0x08, 0x08},
    ['/'] = {0x20, 0x10, 0x08, 0x04, 0x02},

    ['0'] = {0x3E, 0x51, 0x49, 0x45, 0x3E},
    ['1'] = {0x00, 0x42, 0x7F, 0x40, 0x00},
    ['2'] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3'] = {0x21, 0x41, 0x45, 0x4B, 0x31},
    ['4'] = {0x18, 0x14, 0x12, 0x7F, 0x10},
    ['5'] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6'] = {0x3C, 0x4A, 0x49, 0x49, 0x30},
    ['7'] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8'] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9'] = {0x06, 0x49, 0x49, 0x29, 0x1E},

    ['A'] = {0x7E, 0x11, 0x11, 0x11, 0x7E},
    ['B'] = {0x7F, 0x49, 0x49, 0x49, 0x36},
    ['C'] = {0x3E, 0x41, 0x41, 0x41, 0x22},
    ['D'] = {0x7F, 0x41, 0x41, 0x22, 0x1C},
    ['E'] = {0x7F, 0x49, 0x49, 0x49, 0x41},
    ['F'] = {0x7F, 0x09, 0x09, 0x09, 0x01},
    ['G'] = {0x3E, 0x41, 0x49, 0x49, 0x7A},
    ['H'] = {0x7F, 0x08, 0x08, 0x08, 0x7F},
    ['I'] = {0x00, 0x41, 0x7F, 0x41, 0x00},
    ['J'] = {0x20, 0x40, 0x41, 0x3F, 0x01},
    ['K'] = {0x7F, 0x08, 0x14, 0x22, 0x41},
    ['L'] = {0x7F, 0x40, 0x40, 0x40, 0x40},
    ['M'] = {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    ['N'] = {0x7F, 0x04, 0x08, 0x10, 0x7F},
    ['O'] = {0x3E, 0x41, 0x41, 0x41, 0x3E},
    ['P'] = {0x7F, 0x09, 0x09, 0x09, 0x06},
    ['Q'] = {0x3E, 0x41, 0x51, 0x21, 0x5E},
    ['R'] = {0x7F, 0x09, 0x19, 0x29, 0x46},
    ['S'] = {0x46, 0x49, 0x49, 0x49, 0x31},
    ['T'] = {0x01, 0x01, 0x7F, 0x01, 0x01},
    ['U'] = {0x3F, 0x40, 0x40, 0x40, 0x3F},
    ['V'] = {0x1F, 0x20, 0x40, 0x20, 0x1F},
    ['W'] = {0x7F, 0x20, 0x18, 0x20, 0x7F},
    ['X'] = {0x63, 0x14, 0x08, 0x14, 0x63},
    ['Y'] = {0x07, 0x08, 0x70, 0x08, 0x07},
    ['Z'] = {0x61, 0x51, 0x49, 0x45, 0x43},

    ['a'] = {0x20, 0x54, 0x54, 0x54, 0x78},
    ['b'] = {0x7F, 0x48, 0x44, 0x44, 0x38},
    ['c'] = {0x38, 0x44, 0x44, 0x44, 0x20},
    ['d'] = {0x38, 0x44, 0x44, 0x48, 0x7F},
    ['e'] = {0x38, 0x54, 0x54, 0x54, 0x18},
    ['f'] = {0x08, 0x7E, 0x09, 0x01, 0x02},
    ['g'] = {0x0C, 0x52, 0x52, 0x52, 0x3E},
    ['h'] = {0x7F, 0x08, 0x04, 0x04, 0x78},
    ['i'] = {0x00, 0x44, 0x7D, 0x40, 0x00},
    ['j'] = {0x20, 0x40, 0x44, 0x3D, 0x00},
    ['k'] = {0x7F, 0x10, 0x28, 0x44, 0x00},
    ['l'] = {0x00, 0x41, 0x7F, 0x40, 0x00},
    ['m'] = {0x7C, 0x04, 0x18, 0x04, 0x78},
    ['n'] = {0x7C, 0x08, 0x04, 0x04, 0x78},
    ['o'] = {0x38, 0x44, 0x44, 0x44, 0x38},
    ['p'] = {0x7C, 0x14, 0x14, 0x14, 0x08},
    ['q'] = {0x08, 0x14, 0x14, 0x18, 0x7C},
    ['r'] = {0x7C, 0x08, 0x04, 0x04, 0x08},
    ['s'] = {0x48, 0x54, 0x54, 0x54, 0x20},
    ['t'] = {0x04, 0x3F, 0x44, 0x40, 0x20},
    ['u'] = {0x3C, 0x40, 0x40, 0x20, 0x7C},
    ['v'] = {0x1C, 0x20, 0x40, 0x20, 0x1C},
    ['w'] = {0x3C, 0x40, 0x30, 0x40, 0x3C},
    ['x'] = {0x44, 0x28, 0x10, 0x28, 0x44},
    ['y'] = {0x0C, 0x50, 0x50, 0x50, 0x3C},
    ['z'] = {0x44, 0x64, 0x54, 0x4C, 0x44},
};

static esp_err_t oled_reset_gpio(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << OLED_RST_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_RETURN_ON_ERROR(gpio_config(&io_conf), TAG, "Failed to config OLED reset GPIO");

    gpio_set_level(OLED_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    gpio_set_level(OLED_RST_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    gpio_set_level(OLED_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(200));

    return ESP_OK;
}


static void oled_set_pixel(int x, int y, bool on)
{
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT)
    {
        return;
    }

    int byte_index = x + (y / 8) * OLED_WIDTH;
    uint8_t bit_mask = 1 << (y % 8);

    if (on)
    {
        oled_buffer[byte_index] |= bit_mask;
    }
    else
    {
        oled_buffer[byte_index] &= (uint8_t)~bit_mask;
    }
}

static void oled_draw_char(int x, int y, char c)
{
    if ((uint8_t)c >= sizeof(font5x7) / sizeof(font5x7[0]))
    {
        c = ' ';
    }

    const uint8_t *bitmap = font5x7[(uint8_t)c];

    for (int col = 0; col < 5; col++)
    {
        uint8_t line = bitmap[col];

        for (int row = 0; row < 7; row++)
        {
            bool pixel_on = (line & (1 << row)) != 0;
            oled_set_pixel(x + col, y + row, pixel_on);
        }
    }
}

esp_err_t oled_display_init(void)
{
    ESP_RETURN_ON_ERROR(oled_reset_gpio(), TAG, "OLED hardware reset failed");

    vTaskDelay(pdMS_TO_TICKS(200));
    memset(oled_buffer, 0x00, sizeof(oled_buffer));

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = OLED_I2C_PORT,
        .sda_io_num = OLED_SDA_GPIO,
        .scl_io_num = OLED_SCL_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_RETURN_ON_ERROR(
        i2c_new_master_bus(&bus_config, &oled_bus),
        TAG,
        "Failed to create OLED I2C bus");

    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = OLED_ADDR,
        .scl_speed_hz = OLED_I2C_SPEED_HZ,
        .control_phase_bytes = 1,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_bit_offset = 6,
    };

    ESP_RETURN_ON_ERROR(
        esp_lcd_new_panel_io_i2c(oled_bus, &io_config, &oled_io),
        TAG,
        "Failed to create OLED panel IO");

    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = OLED_HEIGHT,
    };

    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .vendor_config = &ssd1306_config,
    };

    ESP_RETURN_ON_ERROR(
        esp_lcd_new_panel_ssd1306(oled_io, &panel_config, &oled_panel),
        TAG,
        "Failed to create SSD1306 panel");

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_reset(oled_panel),
        TAG,
        "Failed to reset SSD1306 panel");

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_init(oled_panel),
        TAG,
        "Failed to init SSD1306 panel");

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_disp_on_off(oled_panel, true),
        TAG,
        "Failed to turn OLED on");

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_invert_color(oled_panel, false),
        TAG,
        "Failed to disable OLED invert color");

    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_mirror(oled_panel, false, false),
        TAG,
        "Failed to configure OLED mirror");

    oled_display_clear();
    ESP_RETURN_ON_ERROR(oled_display_update(), TAG, "Failed to clear OLED");

    ESP_LOGI(TAG, "OLED initialized");

    return ESP_OK;
}

esp_err_t oled_display_clear(void)
{
    memset(oled_buffer, 0x00, sizeof(oled_buffer));
    return ESP_OK;
}

esp_err_t oled_display_draw_text(int x, int y, const char *text)
{
    if (text == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    int cursor_x = x;

    while (*text != '\0')
    {
        if (cursor_x > OLED_WIDTH - 6)
        {
            break;
        }

        oled_draw_char(cursor_x, y, *text);
        cursor_x += 6;
        text++;
    }

    return ESP_OK;
}

esp_err_t oled_display_update(void)
{
    if (oled_panel == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    return esp_lcd_panel_draw_bitmap(
        oled_panel,
        0,
        0,
        OLED_WIDTH,
        OLED_HEIGHT,
        oled_buffer);
}

esp_err_t oled_display_show_text(const char *line1, const char *line2, const char *line3)
{
    ESP_RETURN_ON_ERROR(oled_display_clear(), TAG, "Failed to clear buffer");

    if (line1 != NULL)
    {
        ESP_RETURN_ON_ERROR(oled_display_draw_text(0, 0, line1), TAG, "Failed to draw line 1");
    }

    if (line2 != NULL)
    {
        ESP_RETURN_ON_ERROR(oled_display_draw_text(0, 12, line2), TAG, "Failed to draw line 2");
    }

    if (line3 != NULL)
    {
        ESP_RETURN_ON_ERROR(oled_display_draw_text(0, 24, line3), TAG, "Failed to draw line 3");
    }

    return oled_display_update();
}

esp_err_t oled_display_show_status(const char *title, const char *status, const char *detail)
{
    return oled_display_show_text(title, status, detail);
}

esp_err_t oled_display_show_measurements(const sensor_data_t *data)
{
    if (data == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    char line1[32];
    char line2[32];
    char line3[32];

    if (!data->bme680_ok && !data->light_ok)
    {
        snprintf(line1, sizeof(line1), "Erreur capteurs");
        snprintf(line2, sizeof(line2), "BME680: FAIL");
        snprintf(line3, sizeof(line3), "Light : FAIL");
        return oled_display_show_text(line1, line2, line3);
    }

    if (data->bme680_ok)
    {
        snprintf(line1, sizeof(line1), "T:%.1fC H:%.0f%%",
                 data->temperature_c,
                 data->humidity_pct);

        snprintf(line2, sizeof(line2), "P:%.0f hPa",
                 data->pressure_hpa);
    }
    else
    {
        snprintf(line1, sizeof(line1), "BME680: FAIL");
        snprintf(line2, sizeof(line2), "Temp/Hum/P: --");
    }

    if (data->light_ok)
    {
        snprintf(line3, sizeof(line3), "Lum: %u", data->light_lux);
    }
    else
    {
        snprintf(line3, sizeof(line3), "Light: FAIL");
    }

    return oled_display_show_text(line1, line2, line3);
}

esp_err_t oled_display_sleep(void)
{
    if (oled_panel == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_lcd_panel_disp_on_off(oled_panel, false);

    return ESP_OK;
}
esp_err_t oled_display_shutdown(void)
{
    esp_err_t ret = ESP_OK;

    if (oled_panel != NULL)
    {
        memset(oled_buffer, 0x00, sizeof(oled_buffer));

        ret = esp_lcd_panel_draw_bitmap(
            oled_panel,
            0,
            0,
            OLED_WIDTH,
            OLED_HEIGHT,
            oled_buffer);

        if (ret != ESP_OK)
        {
            ESP_LOGW(TAG, "Failed to clear OLED before shutdown: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(100));

        ret = esp_lcd_panel_disp_on_off(oled_panel, false);
        if (ret != ESP_OK)
        {
            ESP_LOGW(TAG, "Failed to turn OLED off: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(100));

        esp_lcd_panel_del(oled_panel);
        oled_panel = NULL;
    }

    if (oled_io != NULL)
    {
        esp_lcd_panel_io_del(oled_io);
        oled_io = NULL;
    }

    if (oled_bus != NULL)
    {
        i2c_del_master_bus(oled_bus);
        oled_bus = NULL;
    }

    gpio_set_level(OLED_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    ESP_LOGI(TAG, "OLED shutdown complete");

    return ESP_OK;
}
