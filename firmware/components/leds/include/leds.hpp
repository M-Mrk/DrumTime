#pragma once
#include <cstdint>

#include "driver/gpio.h"
#include "driver/i2c_master.h"

enum class CurrentScale : uint8_t {
  FULL = 0,
  HALF = 1,
  THIRD = 2,
  QUARTER = 3,
};

class LedController {
 private:
  uint16_t address;
  gpio_num_t scl_pin;
  gpio_num_t sda_pin;
  i2c_master_bus_handle_t i2c_bus_handle;
  i2c_master_dev_handle_t i2c_dev_handle;

  void setupI2C(uint16_t address, gpio_num_t scl_pin, gpio_num_t sda_pin);
  void setupDefaultSettings();

  void reset();
  void setOutputFreq(bool high_freq);
  void setStateOverwrite(bool enable);
  void updatePWM();
  void setControlAll(CurrentScale current_scale, bool state);

  void setChannelLevel(uint8_t channel, uint8_t level);
  void setChannelLevelAll(uint8_t level);

 public:
  LedController(uint16_t address, gpio_num_t scl_pin, gpio_num_t sda_pin);
  void setRGBIndicatior(uint8_t r, uint8_t g, uint8_t b);
  void setLightBarLevel(uint8_t progress);
};
