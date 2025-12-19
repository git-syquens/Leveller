idf_component_register(
    SRCS "xpt2046.c"
    INCLUDE_DIRS "." "include"
    REQUIRES LCD GUI freertos esp_driver_gpio
)
