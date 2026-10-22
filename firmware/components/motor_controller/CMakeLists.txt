idf_component_register(
  SRCS "src/motor_controller.cpp"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES esp_driver_gpio freertos tmc_bus
  REQUIRES tmc tmc_bus
)
