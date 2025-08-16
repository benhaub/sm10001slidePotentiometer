#The IDF main component must be in the folder named 'main' unless you change the name through advanced settings.
idf_component_register(SRCS
                         "../main.cpp"
                       INCLUDE_DIRS
                         "."
                       PRIV_REQUIRES
                         driver
                         pthread
                         esp_wifi
                         spiffs
                         esp_driver_mcpwm
                         esp_adc
                         esp_app_format
                         mbedtls
                         nvs_flash
                         esp_timer
)