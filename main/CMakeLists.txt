idf_component_register(
    SRCS
        "main.c"
        "drivers/i2c_bus.c"
        "drivers/bme680_sensor.c"
        "drivers/tmg39931_sensor.c"
        "drivers/oled_display.c"
        "drivers/lora_radio.c"
        "include/sensor_data.c"
        "services/sensor_service.c"
        "tasks/sensor_task.c"
        "tasks/display_task.c"
        "tasks/lora_tx_task.c"

    INCLUDE_DIRS
        "."
        "include"
        "drivers"
        "services"
        "tasks"

    REQUIRES
        driver
        esp_driver_i2c
        esp_lcd
        bme68x
)