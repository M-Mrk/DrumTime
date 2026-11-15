#include "clock.hpp"

#include <time.h>

namespace {
struct ClockTime {
  int hours;
  int minutes;
  int seconds;
};

ClockTime getCurrentTime() {
  time_t now = 0;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  return ClockTime{
      .hours = timeinfo.tm_hour,
      .minutes = timeinfo.tm_min,
      .seconds = timeinfo.tm_sec,
  };
}

long map(long val, long in_min, long in_max, long out_min, long out_max) {
  return ((val - in_min) * (out_max - out_min) / (in_max - in_min)) + out_min;
}

int calculateTurnsNeeded(int curDigit, int lastDigit) {
  if (curDigit >= lastDigit) {
    return curDigit - lastDigit;
  }
  return 10 - lastDigit + curDigit;
}

void secondsUpdate(int seconds, LedController& leds) {
  auto progress = static_cast<uint8_t>(map(seconds, 0, 60, 0, 100));
  leds.setLightBarLevel(progress);
}

void updateDigitPair(int curValue, int lastValue, uint8_t leftMotorNum,
                     uint8_t rightMotorNum, MotorController& motors) {
  int rTurnsNeeded = calculateTurnsNeeded(curValue % 10, lastValue % 10);
  for (int i = 0; i < rTurnsNeeded; i++) {
    motors.get_motor(rightMotorNum).turn_forward();
  }

  int lTurnsNeeded = calculateTurnsNeeded(curValue / 10, lastValue / 10);
  for (int i = 0; i < lTurnsNeeded; i++) {
    motors.get_motor(leftMotorNum).turn_forward();
  }
}

void minutesUpdate(int curMinutes, int lastMinutes, MotorController& motors) {
  uint8_t leftMotorNum = 1;
  uint8_t rightMotorNum = 0;
  updateDigitPair(curMinutes, lastMinutes, leftMotorNum, rightMotorNum, motors);
}

void hoursUpdate(int curHours, int lastHours, MotorController& motors) {
  uint8_t leftMotorNum = 3;
  uint8_t rightMotorNum = 2;
  updateDigitPair(curHours, lastHours, leftMotorNum, rightMotorNum, motors);
}
}  // namespace

void startClock(LedController& leds, MotorController& motors) {
  TickType_t lastWake = xTaskGetTickCount();
  ClockTime lastTime{.hours = 0, .minutes = 0, .seconds = 0};

  while (true) {
    vTaskDelayUntil(&lastWake,
                    pdMS_TO_TICKS(1000));  // Wait until next second has passed
    ClockTime curTime = getCurrentTime();
    secondsUpdate(curTime.seconds, leds);
    minutesUpdate(curTime.minutes, lastTime.minutes, motors);
    hoursUpdate(curTime.hours, lastTime.hours, motors);

    lastTime = curTime;
  }
}
