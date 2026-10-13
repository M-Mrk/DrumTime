#include <leds.hpp>
#include <registers.hpp>

#include "esp_log.h"

constexpr static const char* TAG = "LEDs/logic";

void LedController::setupDefaultSettings() {
  this->reset();
  this->setControlAll(CurrentScale::QUARTER, false);
}

LedController::LedController(uint16_t address, gpio_num_t scl_pin,
                             gpio_num_t sda_pin)
    : address(address), scl_pin(scl_pin), sda_pin(sda_pin) {
  this->setupI2C(address, scl_pin, sda_pin);
  this->setupDefaultSettings();
}

void LedController::setRGBIndicatior(uint8_t red, uint8_t green, uint8_t blue) {
  const int r_channel_register = PWM_REGISTERS_START + 33;  // Register num 34
  const int g_channel_register = r_channel_register + 1;
  const int b_channel_register = g_channel_register + 1;
  this->setChannelLevel(r_channel_register, red);
  this->setChannelLevel(g_channel_register, green);
  this->setChannelLevel(b_channel_register, blue);
  this->updatePWM();
}

constexpr const int NUM_LEDS = 33;
constexpr const int NUM_PWM_STEPS = 256;
constexpr const int max_steps = NUM_LEDS * NUM_PWM_STEPS;
void LedController::setLightBarLevel(uint8_t progress) {
  // TODO: Use gamma correction if it doesn't look right
  // TODO: Increase performance by using 1 transaction for full update if its
  // too slow
  if (progress > 100) {
    ESP_LOGW(TAG,
             "Can't set light bar progress to anything over 100 percent, "
             "defaulting to 100");
    progress = 100;
  }

  int current_step = max_steps / 100 * progress;
  int current_channel = 0;

  // set full brightness LEDs
  int num_full_leds = current_step / NUM_PWM_STEPS;
  for (int i = 0; i < num_full_leds; i++) {
    this->setChannelLevel(current_channel, 255);
    current_channel += 1;
  }

  // set transitioning LED
  int remaining_steps = current_step - (num_full_leds * NUM_PWM_STEPS);
  if (remaining_steps != 0) {
    this->setChannelLevel(current_channel, remaining_steps);
    current_channel += 1;
  }

  // set turned off LEDs
  while (current_channel < NUM_LEDS) {
    this->setChannelLevel(current_channel, 0);
    current_channel += 1;
  }
  this->updatePWM();
}
