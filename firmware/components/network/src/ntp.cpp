#include "esp_log.h"
#include "esp_sntp.h"
#include "network.hpp"

constexpr const char* TAG = "network/ntp";
esp_err_t Networking::syncTime() {
  ESP_LOGI(TAG, "Starting NTP synchronisation");

  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");

  esp_sntp_init();

  const int max_retry = 12;
  const int retry_delay_ms = 2000;
  int retry = 0;

  while (esp_sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED &&
         retry < max_retry) {
    ESP_LOGI(TAG, "Awaiting NTP synchronisation... Try %d of %d", retry,
             max_retry);
    vTaskDelay(pdMS_TO_TICKS(retry_delay_ms));
    retry++;
  }

  esp_sntp_stop();

  if (retry >= max_retry) {
    ESP_LOGE(TAG, "NTP synchronisation timed out!");
    return ESP_ERR_TIMEOUT;
  }

  ESP_LOGI(TAG, "Finished NTP synchronisation");
  return ESP_OK;
}
