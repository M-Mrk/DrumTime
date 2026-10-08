#pragma once
#include <cstdint>

#include "driver/gpio.h"

class LedController {
 private:
  uint16_t address;
  gpio_num_t scl_pin;
  gpio_num_t sda_pin;

  void setChannelLevel(uint8_t channel, uint8_t level);

 public:
  LedController(uint16_t address, gpio_num_t scl_pin, gpio_num_t sda_pin);
  void setRGBIndicatior(uint8_t r, uint8_t g, uint8_t b);
  void setLightBarLevel(uint8_t progress);
};
