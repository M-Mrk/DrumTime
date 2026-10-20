#pragma once
#include <cstdint>
#include <tmc_bus.hpp>

#include "stepper_motor_encoder.h"
#include "types.hpp"

class Tmc {
 private:
  TmcBus& bus;
  uint8_t address;
  gpio_num_t step;
  gpio_num_t dir;

  rmt_channel_handle_t rmt_chan;
  rmt_encoder_handle_t acc_enc_handle;
  rmt_encoder_handle_t uni_enc_handle;
  rmt_encoder_handle_t dec_enc_handle;

  char tag[16];

  esp_err_t writeWithReadBack(uint8_t register_addr, uint32_t value);
  esp_err_t writeWithIFCounterCheck(uint8_t register_addr, uint32_t value);
  esp_err_t getIOIN(TmcIOIN_t& output_buffer);
  esp_err_t getIFCNT(uint8_t& buffer);

  esp_err_t rotateOnce();

 public:
  Tmc(TmcBus& bus, uint8_t address, gpio_num_t step, gpio_num_t dir);
  esp_err_t init();

  esp_err_t setGconf(TmcGconf_t gconf);
  esp_err_t setNodeConf(uint8_t delay);
  esp_err_t setIHoldIRun(IHoldIRun_t conf);
  esp_err_t setChopConf(ChopConf_t conf);

  esp_err_t home();
  esp_err_t turn_forward();
  esp_err_t turn_backward();
};
