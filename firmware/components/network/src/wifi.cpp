#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "network.hpp"
#include "nvs_flash.h"

constexpr const char* TAG = "network/wifi";

constexpr const int ESP_MAXIMUM_RETRY = 12;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
void Networking::eventHandler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
  auto* net = static_cast<Networking*>(arg);

  static int s_retry_num = 0;
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < ESP_MAXIMUM_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    } else {
      xEventGroupSetBits(net->wifi_event_handle, WIFI_FAIL_BIT);
    }
    ESP_LOGI(TAG, "connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    auto* event = static_cast<ip_event_got_ip_t*>(event_data);
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(net->wifi_event_handle, WIFI_CONNECTED_BIT);
  }
}

esp_err_t Networking::init() {
  // NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // WiFi Stack
  wifi_event_handle = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_netif_init());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandler, this, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &eventHandler, this, &instance_got_ip));

  return ESP_OK;
}

esp_err_t Networking::connectWifi(std::array<uint8_t, 32> ssid,
                                  std::array<uint8_t, 32> password) {
  wifi_config_t wifi_conf = {};
  memcpy(wifi_conf.sta.ssid, ssid.data(), ssid.size());
  memcpy(wifi_conf.sta.password, password.data(), password.size());

  wifi_conf.sta.threshold = {
      .rssi = 0,
      .authmode = WIFI_AUTH_WPA2_WPA3_PSK,
      .rssi_5g_adjustment = 0,
  };
  wifi_conf.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_conf));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_sta finished.");

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or
   * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT). The
   * bits are set by event_handler() (see above) */
  EventBits_t bits =
      xEventGroupWaitBits(wifi_event_handle, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                          pdFALSE, pdFALSE, portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we
   * can test which event actually happened. */
  if (WIFI_CONNECTED_BIT & bits) {
    ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ssid, password);
  } else if (WIFI_FAIL_BIT & bits) {
    ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid, password);
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
  }

  return ESP_OK;
}
