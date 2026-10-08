#include <leds.hpp>

#include "driver/gpio.h"

namespace pins {
constexpr gpio_num_t kLedScl = GPIO_NUM_21;
constexpr gpio_num_t kLedSda = GPIO_NUM_47;
}  // namespace pins

namespace {
constexpr uint16_t kLedI2cAddress = 0x40;
}

extern "C" void app_main(void) {
  static LedController leds{kLedI2cAddress, pins::kLedScl, pins::kLedSda};
}
