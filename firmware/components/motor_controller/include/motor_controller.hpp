#pragma once
#include <array>

#include "tmc.hpp"

struct MotorIO {
  uint8_t address;
  gpio_num_t step;
  gpio_num_t dir;
};

class MotorController {
 private:
  gpio_num_t enn_pin;
  bool all_intialised = false;

  TmcBus bus;
  std::array<Tmc, 4> motors;

 public:
  MotorController(TmcBus& bus, MotorIO m_io0, MotorIO m_io1, MotorIO m_io2,
                  MotorIO m_io3, gpio_num_t enn_pin);
  esp_err_t init();
  esp_err_t setENN(bool level);
  Tmc& get_motor(uint8_t num);
};
