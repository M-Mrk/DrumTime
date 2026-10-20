#include "tmc.hpp"

#include <cstdio>

#include "driver/rmt_tx.h"
#include "esp_check.h"
#include "esp_log.h"

esp_err_t Tmc::getIOIN(TmcIOIN_t& output_buffer) {
  uint32_t buffer = 0;
  esp_err_t result = bus.read(address, 0x06, buffer);
  output_buffer = TmcIOIN_t{buffer};
  return result;
}

constexpr const int RESOLUTION_HZ = 1000000;  // 1 MHz
Tmc::Tmc(TmcBus& bus, uint8_t address, gpio_num_t step, gpio_num_t dir)
    : bus(bus), address(address), step(step), dir(dir) {
  std::snprintf(tag, sizeof(tag), "TMC (0x%02X)", address);

  // gpio setup
  gpio_config_t dir_gpio_conf = {
      .pin_bit_mask = 1ULL << dir,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  ESP_ERROR_CHECK(gpio_config(&dir_gpio_conf));

  // rmt
  rmt_tx_channel_config_t tx_conf = {
      .gpio_num = step,
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = RESOLUTION_HZ,
      .mem_block_symbols = 64,
      .trans_queue_depth = 10,
      .intr_priority = 0,
      .flags = {},
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_conf, &rmt_chan));
  ESP_ERROR_CHECK(rmt_enable(rmt_chan));

  // motor curve
  /// accelerate
  stepper_motor_curve_encoder_config_t acc_curve_conf = {
      .resolution = RESOLUTION_HZ,
      .sample_points = 500,
      .start_freq_hz = 1000,
      .end_freq_hz = 6400,
  };
  ESP_ERROR_CHECK(
      rmt_new_stepper_motor_curve_encoder(&acc_curve_conf, &acc_enc_handle));

  /// Uniform
  stepper_motor_uniform_encoder_config_t uni_curve_conf = {
      .resolution = RESOLUTION_HZ,
  };
  ESP_ERROR_CHECK(
      rmt_new_stepper_motor_uniform_encoder(&uni_curve_conf, &uni_enc_handle));

  /// decelerate
  stepper_motor_curve_encoder_config_t dec_curve_conf = {
      .resolution = RESOLUTION_HZ,
      .sample_points = 500,
      .start_freq_hz = 6400,
      .end_freq_hz = 1000,
  };
  ESP_ERROR_CHECK(
      rmt_new_stepper_motor_curve_encoder(&dec_curve_conf, &dec_enc_handle));
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

esp_err_t Tmc::rotateOnce() {
  const static uint32_t total = 3168;
  const static uint32_t ramp = 500;
  const static uint32_t uniform_speed_hz = 6400;
  const static uint32_t cruise_steps = total - (2 * ramp);

  rmt_transmit_config_t once = {
      .loop_count = 0,
      .flags = {},
  };
  rmt_transmit_config_t loop = {
      .loop_count = cruise_steps,
      .flags = {},
  };

  // accel phase
  ESP_ERROR_CHECK(
      rmt_transmit(rmt_chan, acc_enc_handle, &ramp, sizeof(ramp), &once));

  // uniform phase
  ESP_ERROR_CHECK(rmt_transmit(rmt_chan, uni_enc_handle, &uniform_speed_hz,
                               sizeof(uniform_speed_hz), &loop));

  // decel phase
  ESP_ERROR_CHECK(
      rmt_transmit(rmt_chan, dec_enc_handle, &ramp, sizeof(ramp), &once));

  return ESP_OK;
}

esp_err_t Tmc::turn_forward() {
  gpio_set_level(dir, 0);

  return rotateOnce();
}

esp_err_t Tmc::turn_backward() {
  gpio_set_level(dir, 1);

  return rotateOnce();
}
