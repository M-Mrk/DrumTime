#pragma once
#include <array>

#include "esp_err.h"
#include "esp_event_base.h"
#include "freertos/idf_additions.h"

class Networking {
 private:
  EventGroupHandle_t wifi_event_handle;
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;

  static void eventHandler(void* arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data);

 public:
  esp_err_t init();
  esp_err_t connectWifi(std::array<uint8_t, 32> ssid,
                        std::array<uint8_t, 32> password);
  esp_err_t syncTime();
};
