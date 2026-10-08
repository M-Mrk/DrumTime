idf_component_register(
  SRCS "src/leds.cpp"
  INCLUDE_DIRS "include"
  REQUIRES esp_driver_gpio, esp_driver_i2c
)
