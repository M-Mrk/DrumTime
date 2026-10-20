idf_component_register(
  SRCS "stepper_motor_encoder.c"
  INCLUDE_DIRS "."
  PRIV_REQUIRES esp_driver_rmt esp_driver_gpio
)
