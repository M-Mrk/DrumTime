#include <cstdint>
#include <leds.hpp>

#include "clock.hpp"
#include "driver/gpio.h"
#include "esp_log.h"
#include "motor_controller.hpp"
#include "network.hpp"

namespace pins {
constexpr gpio_num_t kLedScl = GPIO_NUM_21;
constexpr gpio_num_t kLedSda = GPIO_NUM_47;

constexpr gpio_num_t kM0Hall = GPIO_NUM_4;
constexpr gpio_num_t kM0Dir = GPIO_NUM_5;
constexpr gpio_num_t kM0Step = GPIO_NUM_6;

constexpr gpio_num_t kM1Hall = GPIO_NUM_7;
constexpr gpio_num_t kM1Dir = GPIO_NUM_15;
constexpr gpio_num_t kM1Step = GPIO_NUM_16;

constexpr gpio_num_t kM2Hall = GPIO_NUM_8;
constexpr gpio_num_t kM2Dir = GPIO_NUM_9;
constexpr gpio_num_t kM2Step = GPIO_NUM_10;

constexpr gpio_num_t kM3Hall = GPIO_NUM_11;
constexpr gpio_num_t kM3Dir = GPIO_NUM_12;
constexpr gpio_num_t kM3Step = GPIO_NUM_13;

constexpr gpio_num_t kMEnn = GPIO_NUM_14;

constexpr gpio_num_t kUartTx = GPIO_NUM_17;
constexpr gpio_num_t kUartRx = GPIO_NUM_18;

constexpr gpio_num_t kLdr = GPIO_NUM_1;
}  // namespace pins

namespace {
constexpr uint32_t kSlowStartDurationMs = 60;
constexpr uint16_t kLedI2cAddress = 0x40;

void time_sync_task(void* pvParameters) {
  auto* net = static_cast<Networking*>(pvParameters);
  if (net == nullptr) {
    ESP_LOGE("time sync task", "Didn't get network passed!");
    vTaskDelete(nullptr);
  }

  while (true) {
    net->syncTime();
    vTaskDelay(pdMS_TO_TICKS(86400000));  // Wait 1 day
  }
}
}  // namespace

extern "C" void app_main(void) {
  // Wait to make sure board is fully powered to lessen the chance of a brownout
  vTaskDelay(pdMS_TO_TICKS(kSlowStartDurationMs));

  static LedController leds{kLedI2cAddress, pins::kLedScl, pins::kLedSda};
  leds.setRGBIndicatior(0, 205, 80);

  static TmcBus motor_bus{UART_NUM_1, pins::kUartRx, pins::kUartTx};
  static MotorController motors{
      motor_bus,
      MotorIO{.address = 0, .step = pins::kM0Step, .dir = pins::kM0Dir},
      MotorIO{.address = 1, .step = pins::kM1Step, .dir = pins::kM1Dir},
      MotorIO{.address = 2, .step = pins::kM2Step, .dir = pins::kM2Dir},
      MotorIO{.address = 3, .step = pins::kM3Step, .dir = pins::kM3Dir},
      pins::kMEnn};
  ESP_ERROR_CHECK(motors.init());
  ESP_ERROR_CHECK(motors.setENN(false));
  leds.setRGBIndicatior(0, 220, 30);

  static Networking network{};
  ESP_ERROR_CHECK(network.init());
  ESP_ERROR_CHECK(network.connectWifi(
      std::array<uint8_t, 32>{'w', 'i', 'f', 'i', '-', 't', 'e', 's', 't'},
      std::array<uint8_t, 32>{'c', 'h', 'a', 'n', 'g', 'e', 'm', 'e', '!'}));
  ESP_ERROR_CHECK(network.syncTime());

  xTaskCreate(time_sync_task, "Time sync task", 2048, &network,
              tskIDLE_PRIORITY + 1, nullptr);
  ESP_LOGI("firmware", "Finished initalising!");
  startClock(leds, motors);
}
