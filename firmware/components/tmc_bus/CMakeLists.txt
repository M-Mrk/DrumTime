idf_component_register(
  SRCS "src/tmc_bus.cpp"
  INCLUDE_DIRS "include"
  REQUIRES esp_driver_gpio esp_driver_uart freertos
)
