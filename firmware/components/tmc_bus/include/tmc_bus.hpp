#pragma once
#include <cstdint>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class TmcBus {
 private:
  SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
  uart_port_t uart_num;

  void setCrc(uint8_t* datagram, size_t datagram_size);
  void setupUart(gpio_num_t rx_pin, gpio_num_t tx_pin);
  esp_err_t sendAndConsumeEcho(uint8_t* data, size_t data_size);

 public:
  TmcBus(uart_port_t uart_num, gpio_num_t rx_pin, gpio_num_t tx_pin);
  esp_err_t write(uint8_t node_addr, uint8_t register_addr, uint32_t data);
  esp_err_t read(uint8_t node_addr, uint8_t register_addr,
                 uint32_t& output_buffer);
};
