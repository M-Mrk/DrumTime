#pragma once

constexpr const int NUM_CHANNELS = 36;

constexpr const int SHUTDOWN_REGISTER = 0x00;
constexpr const int PWM_REGISTERS_START = 0x01;
constexpr const int PWM_UPDATE_REGISTER = 0x25;
constexpr const int LED_CONTROL_REGISTERS_START = 0x26;
constexpr const int GLOBAL_CONTROL_REGISTER = 0x4A;
constexpr const int OUTPUT_FREQ_REGISTER = 0x4B;
constexpr const int RESET_REGISTER = 0x4F;
