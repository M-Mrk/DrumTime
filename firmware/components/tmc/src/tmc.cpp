#include "tmc.hpp"

#include <cstdio>
#include <tmc_bus.hpp>

#include "esp_check.h"
#include "esp_log.h"

esp_err_t Tmc::getIOIN(TmcIOIN_t& output_buffer) {
  uint32_t buffer = 0;
  esp_err_t result = bus.read(address, 0x06, buffer);
  output_buffer = TmcIOIN_t{buffer};
  return result;
}

Tmc::Tmc(TmcBus& bus, uint8_t address) : bus(bus), address(address) {
  std::snprintf(tag, sizeof(tag), "TMC (0x%02X)", address);
}

esp_err_t Tmc::init() {
  TmcIOIN_t ioin;
  ESP_RETURN_ON_ERROR(getIOIN(ioin), tag, "IOIN");

  uint8_t version = ioin.version();
  if (version != 0x21) {
    ESP_LOGE(tag, "Got unexpected version: 0x%02X", version);
    return ESP_ERR_INVALID_VERSION;
  }
  return ESP_OK;
}

esp_err_t Tmc::getIFCNT(uint8_t& buffer) {
  uint32_t big_buffer = 0;
  ESP_RETURN_ON_ERROR(bus.read(address, 0x02, big_buffer), tag, "read");
  buffer = static_cast<uint8_t>(big_buffer);
  return ESP_OK;
}

// Use to validate requests that write to registers, shich can also be read from
esp_err_t Tmc::writeWithReadBack(uint8_t register_addr, uint32_t value) {
  ESP_RETURN_ON_ERROR(bus.write(address, register_addr, value), tag, "write");

  uint32_t output_buffer = 0;
  ESP_RETURN_ON_ERROR(bus.read(address, register_addr, output_buffer), tag,
                      "read");

  if (output_buffer != value) {
    ESP_LOGE(tag, "Written value was not returned on read");
    return ESP_ERR_INVALID_RESPONSE;
  }
  return ESP_OK;
}

// IFCNT increases by 1 on successful transaction, so use this to validate
// requests that write to registers that don't support reading
esp_err_t Tmc::writeWithIFCounterCheck(uint8_t register_addr, uint32_t value) {
  uint8_t prior_ifcnt = 0;
  ESP_RETURN_ON_ERROR(getIFCNT(prior_ifcnt), tag, "ifcnt");

  ESP_RETURN_ON_ERROR(bus.write(address, register_addr, value), tag, "write");

  uint8_t after_ifcnt = 0;
  ESP_RETURN_ON_ERROR(getIFCNT(after_ifcnt), tag, "ifcnt");
  if (prior_ifcnt == 255 && after_ifcnt == 0) {
    return ESP_OK;
  }
  if (after_ifcnt == prior_ifcnt + 1) {
    return ESP_OK;
  }
  return ESP_ERR_INVALID_RESPONSE;
}

esp_err_t Tmc::setGconf(TmcGconf_t gconf) {
  return writeWithReadBack(0x00, gconf);
}

esp_err_t Tmc::setNodeConf(uint8_t delay) {
  return writeWithIFCounterCheck(0x03, delay << 8);
}

esp_err_t Tmc::setIHoldIRun(IHoldIRun_t conf) {
  return writeWithIFCounterCheck(0x10, conf);
}

esp_err_t Tmc::setChopConf(ChopConf_t conf) {
  return writeWithReadBack(0x6C, conf);
}
