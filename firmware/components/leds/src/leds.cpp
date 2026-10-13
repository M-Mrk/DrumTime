#include <array>
#include <leds.hpp>
#include <registers.hpp>

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

constexpr static const char* TAG = "LEDs";

constexpr static const uint32_t I2C_FREQ_HZ = 100000;  // 100kHz
constexpr static const int I2C_TIMEOUT = pdMS_TO_TICKS(50);

void LedController::setupI2C(uint16_t address, gpio_num_t scl_pin,
                             gpio_num_t sda_pin) {
  // Bus setup
  ESP_LOGI(TAG, "Setting up I2C");
  i2c_master_bus_handle_t i2c_bus_handle = nullptr;
  i2c_master_bus_config_t i2c_bus_settings = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = sda_pin,
      .scl_io_num = scl_pin,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,
      .trans_queue_depth = 0,
      .flags = {},
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_settings, &i2c_bus_handle));
  this->i2c_bus_handle = i2c_bus_handle;
  ESP_LOGI(TAG, "Set up i2c bus, now setting up device i2c");

  // Device setup
  i2c_master_dev_handle_t i2c_dev_handle = nullptr;
  i2c_device_config_t i2c_dev_settings = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = address,
      .scl_speed_hz = I2C_FREQ_HZ,
      .scl_wait_us = 0,
      .flags = {},
  };
  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &i2c_dev_settings,
                                            &i2c_dev_handle));
  this->i2c_dev_handle = i2c_dev_handle;
  ESP_LOGI(TAG, "Set up device i2c, now testing communication");

  // test communication
  esp_err_t response =
      i2c_master_transmit(this->i2c_dev_handle, nullptr, 0, I2C_TIMEOUT);
  if (response == ESP_OK) {
    ESP_LOGI(TAG, "Successful response, finished setting up connection");
  } else {
    ESP_LOGE(TAG, "Failed to get response from LED driver IC... Restarting");
    abort();
  }
}

void LedController::reset() {
  std::array<uint8_t, 2> write_buffer;

  write_buffer.at(0) = RESET_REGISTER;
  write_buffer.at(1) = 0;

  ESP_ERROR_CHECK_WITHOUT_ABORT(
      i2c_master_transmit(this->i2c_dev_handle, write_buffer.data(),
                          write_buffer.size(), I2C_TIMEOUT));
}

void LedController::setOutputFreq(bool high_freq) {
  std::array<uint8_t, 2> write_buffer;

  write_buffer.at(0) = OUTPUT_FREQ_REGISTER;
  write_buffer.at(1) = static_cast<uint8_t>(high_freq);

  ESP_ERROR_CHECK_WITHOUT_ABORT(
      i2c_master_transmit(this->i2c_dev_handle, write_buffer.data(),
                          write_buffer.size(), I2C_TIMEOUT));
}

// If set to true all LEDs will be off until the Overwrite is cleared again by
// running this function with false
void LedController::setStateOverwrite(bool enable) {
  std::array<uint8_t, 2> write_buffer;

  write_buffer.at(0) = GLOBAL_CONTROL_REGISTER;
  write_buffer.at(1) = static_cast<uint8_t>(enable);

  ESP_ERROR_CHECK_WITHOUT_ABORT(
      i2c_master_transmit(this->i2c_dev_handle, write_buffer.data(),
                          write_buffer.size(), I2C_TIMEOUT));
}

void LedController::updatePWM() {
  std::array<uint8_t, 2> write_buffer;

  write_buffer.at(0) = PWM_UPDATE_REGISTER;
  write_buffer.at(1) = 0;

  ESP_ERROR_CHECK_WITHOUT_ABORT(
      i2c_master_transmit(this->i2c_dev_handle, write_buffer.data(),
                          write_buffer.size(), I2C_TIMEOUT));
}

void LedController::setControlAll(CurrentScale current_scale, bool state) {
  // Array: 1. control register addr + 36x channel control setting
  // IC auto increments address after each byte
  std::array<uint8_t, NUM_CHANNELS + 1> write_buffer;

  // Bit 0: Channel state (on/off)
  // Bit 1-2: Current scaling
  // Other bits are unsued
  uint8_t data_w_state = static_cast<uint8_t>(state) << 0;
  uint8_t data_w_current = static_cast<uint8_t>(current_scale) << 1;
  uint8_t led_control_data = data_w_current | data_w_state;

  // Populate buffer and set address
  write_buffer.fill(led_control_data);
  write_buffer.at(0) = LED_CONTROL_REGISTERS_START;

  ESP_ERROR_CHECK_WITHOUT_ABORT(
      i2c_master_transmit(this->i2c_dev_handle, write_buffer.data(),
                          write_buffer.size(), I2C_TIMEOUT));
}

void LedController::setupDefaultSettings() {
  this->reset();
  this->setControlAll(CurrentScale::QUARTER, false);
}

void LedController::setChannelLevel(uint8_t channel, uint8_t level) {
  if (channel > NUM_CHANNELS - 1) {
    channel = NUM_CHANNELS - 1;
    ESP_LOGW(TAG,
             "Can not update given channel, as there are not that many. "
             "Defaulting to last channel");
  }
  int channel_register = PWM_REGISTERS_START + channel;

  std::array<uint8_t, 2> write_buffer;
  write_buffer.at(0) = channel_register;
  write_buffer.at(1) = level;

  ESP_ERROR_CHECK_WITHOUT_ABORT(
      i2c_master_transmit(this->i2c_dev_handle, write_buffer.data(),
                          write_buffer.size(), I2C_TIMEOUT));
}

void LedController::setChannelLevelAll(uint8_t level) {
  std::array<uint8_t, NUM_CHANNELS + 1> write_buffer;
  write_buffer.fill(level);
  write_buffer.at(0) = PWM_REGISTERS_START;

  ESP_ERROR_CHECK_WITHOUT_ABORT(
      i2c_master_transmit(this->i2c_dev_handle, write_buffer.data(),
                          write_buffer.size(), I2C_TIMEOUT));
}

LedController::LedController(uint16_t address, gpio_num_t scl_pin,
                             gpio_num_t sda_pin)
    : address(address), scl_pin(scl_pin), sda_pin(sda_pin) {
  this->setupI2C(address, scl_pin, sda_pin);
  this->setupDefaultSettings();
}
