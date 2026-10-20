idf_component_register(
  SRCS "src/tmc.cpp"
  INCLUDE_DIRS "include"
  REQUIRES esp_driver_gpio esp_driver_mcpwm freertos tmc_bus
)
