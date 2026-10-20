#pragma once
#include <cstdint>
#include <tmc_bus.hpp>

#include "types.hpp"

class Tmc {
 private:
  TmcBus& bus;
  uint8_t address;
  char tag[16];

  esp_err_t writeWithReadBack(uint8_t register_addr, uint32_t value);
  esp_err_t writeWithIFCounterCheck(uint8_t register_addr, uint32_t value);
  esp_err_t getIOIN(TmcIOIN_t& output_buffer);
  esp_err_t getIFCNT(uint8_t& buffer);

 public:
  Tmc(TmcBus& bus, uint8_t address);
  esp_err_t init();

  esp_err_t setGconf(TmcGconf_t gconf);
  esp_err_t setNodeConf(uint8_t delay);
  esp_err_t setIHoldIRun(IHoldIRun_t conf);
  esp_err_t setChopConf(ChopConf_t conf);

  esp_err_t home();
  esp_err_t turn_forward();
  esp_err_t turn_backwards();
};
