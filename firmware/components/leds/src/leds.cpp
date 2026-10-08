#include <leds.hpp>

LedController::LedController(uint16_t address, gpio_num_t scl_pin,
                             gpio_num_t sda_pin)
    : address(address), scl_pin(scl_pin), sda_pin(sda_pin) {}
