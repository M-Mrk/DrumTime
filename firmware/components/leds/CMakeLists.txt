idf_component_register(
  SRCS "src/wrappers.cpp" "src/logic.cpp"
  INCLUDE_DIRS "include"
  REQUIRES esp_driver_gpio esp_driver_i2c freertos
)
