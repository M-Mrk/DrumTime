idf_component_register(
  SRCS "src/tmc.cpp" "src/wrappers.cpp"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES esp_driver_gpio esp_driver_mcpwm freertos tmc_bus esp_stepper_motor esp_driver_rmt
)
