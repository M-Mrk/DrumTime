idf_component_register(
  SRCS "src/wifi.cpp" "src/ntp.cpp"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES esp_wifi esp_netif esp_event nvs_flash
)
