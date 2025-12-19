idf_component_register(
    SRCS "lcd.c"
    INCLUDE_DIRS "." "include"
    REQUIRES freertos esp_driver_gpio esp_driver_spi
)
