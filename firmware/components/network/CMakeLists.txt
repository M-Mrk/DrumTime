idf_component_register(
  SRCS "src/wifi.cpp" "src/ntp.cpp"
  INCLUDE_DIRS "include"
  REQUIRES esp_event
  PRIV_REQUIRES esp_wifi esp_netif nvs_flash
)
