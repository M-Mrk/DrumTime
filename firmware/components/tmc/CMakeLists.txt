idf_component_register(
  SRCS "src/tmc.cpp" "src/wrappers.cpp"
  INCLUDE_DIRS "include"
  REQUIRES esp_driver_gpio freertos tmc_bus esp_stepper_motor esp_driver_rmt
)
