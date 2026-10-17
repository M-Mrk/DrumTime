#include <array>
#include <cstring>
#include <tmc_bus.hpp>

#include "driver/uart.h"
#include "esp_check.h"
#include "esp_log.h"

constexpr const char* TAG = "TmcBus";

constexpr const int MAX_MUTEX_TIMEOUT = pdMS_TO_TICKS(100);
class MutexGuard {
 private:
  SemaphoreHandle_t mutex;
  esp_err_t take_result;

 public:
  explicit MutexGuard(SemaphoreHandle_t mutex) : mutex(mutex) {
    if (xSemaphoreTake(mutex, MAX_MUTEX_TIMEOUT) != pdTRUE) {
      take_result = ESP_ERR_TIMEOUT;
    } else {
      take_result = ESP_OK;
    }
  };

  esp_err_t check() {
    if (take_result != ESP_OK) {
      ESP_LOGE(TAG, "Couldn't get the mutex. Something is holding the line.");
    }
    return take_result;
  };

  ~MutexGuard() {
    if (take_result != ESP_OK) {
      return;
    }
    xSemaphoreGive(mutex);
  };

  MutexGuard(const MutexGuard&) = delete;
  MutexGuard& operator=(const MutexGuard&) = delete;
};

void TmcBus::setup_uart(gpio_num_t rx_pin, gpio_num_t tx_pin) {
  uart_config_t uart_conf = {};
  uart_conf.baud_rate = 115200;
  uart_conf.data_bits = UART_DATA_8_BITS;
  uart_conf.parity = UART_PARITY_DISABLE;
  uart_conf.stop_bits = UART_STOP_BITS_1;
  uart_conf.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_conf));

  // set pins
  ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_pin, rx_pin));

  // install driver
  ESP_ERROR_CHECK(uart_driver_install(uart_num, 256, 0, 0, nullptr, 0));

  ESP_LOGI(TAG, "Finished setting up UART");
}

TmcBus::TmcBus(uart_port_t uart_num, gpio_num_t rx_pin, gpio_num_t tx_pin)
    : uart_num(uart_num) {
  ESP_LOGI(TAG, "Setting up TmcBus");
  this->setup_uart(rx_pin, tx_pin);
}

void TmcBus::set_crc(uint8_t* datagram, size_t datagram_size) {
  // Taken from tmc2209 datasheet and slightly adjusted
  int i = 0;
  int j = 0;

  uint8_t* crc =
      datagram + (datagram_size - 1);  // CRC located in last byte of message
  uint8_t currentByte = 0;
  *crc = 0;
  for (i = 0; i < (datagram_size - 1);
       i++) {                   // Execute for all bytes of a message
    currentByte = datagram[i];  // Retrieve a byte to be sent from Array
    for (j = 0; j < 8; j++) {
      if ((*crc >> 7) ^
          (currentByte & 0x01))  // update CRC based result of XOR operation
      {
        *crc = (*crc << 1) ^ 0x07;
      } else {
        *crc = (*crc << 1);
      }
      currentByte = currentByte >> 1;
    }  // for CRC bit
  }  // for message byte
}

constexpr const uint8_t SYNC_NIBBLE = 0b00000101;
constexpr const int MAX_ECHO_TIMEOUT = pdMS_TO_TICKS(50);

// Sends data, awaits echo and checks integrity
esp_err_t TmcBus::send_and_consume_echo(uint8_t* data, size_t data_size) {
  uart_flush_input(uart_num);
  if (uart_write_bytes(uart_num, data, data_size) !=
      static_cast<int>(data_size)) {
    return ESP_FAIL;
  }

  std::array<uint8_t, 8> echo_data;
  if (uart_read_bytes(uart_num, echo_data.data(), data_size,
                      MAX_ECHO_TIMEOUT) != static_cast<int>(data_size)) {
    return ESP_FAIL;
  }

  if (std::memcmp(echo_data.data(), data, data_size) != 0) {
    return ESP_FAIL;
  }
  return ESP_OK;
};

esp_err_t TmcBus::write(uint8_t node_addr, uint8_t register_addr,
                        uint32_t data) {
  // Sync&reserve, node_addr, register_addr&write flag + 32bits data + CRC
  std::array<uint8_t, 3 + (32 / 8) + 1> write_buffer;
  write_buffer.at(0) = SYNC_NIBBLE;
  write_buffer.at(1) = node_addr;

  // 7 bit register address and bit 7 is 1 for writing
  write_buffer.at(2) = (register_addr & 0x7F) | 0x80;

  // copy 32bit data with highest byte first and bytes in LSB to MSB order.
  int buffer_index = 3;
  for (int i = (32 / 8); i > 0; i--) {
    int shift_by = (i - 1) * 8;
    uint8_t extracted_byte = (data >> shift_by) & 0xFF;
    write_buffer.at(buffer_index) = extracted_byte;
    buffer_index += 1;
  }

  set_crc(write_buffer.data(), write_buffer.size());

  MutexGuard lock(mutex);
  ESP_RETURN_ON_ERROR(lock.check(), TAG, "mutex");
  return send_and_consume_echo(write_buffer.data(), write_buffer.size());
}

constexpr const int MAX_RESPONSE_TIMEOUT = pdMS_TO_TICKS(50);
esp_err_t TmcBus::read(uint8_t node_addr, uint8_t register_addr,
                       uint32_t& output_buffer) {
  // Send request
  // Sync&reserve, node_addr, register_addr&write flag + CRC
  std::array<uint8_t, 4> write_buffer;
  write_buffer.at(0) = SYNC_NIBBLE;
  write_buffer.at(1) = node_addr;

  // 7 bit register address and bit 7 is 0 for reading
  write_buffer.at(2) = register_addr & 0x7F;

  set_crc(write_buffer.data(), write_buffer.size());

  MutexGuard lock(mutex);
  ESP_RETURN_ON_ERROR(lock.check(), TAG, "mutex");

  ESP_RETURN_ON_ERROR(
      send_and_consume_echo(write_buffer.data(), write_buffer.size()), TAG,
      "request");

  // Read response
  std::array<uint8_t, 3 + (32 / 8) + 1> response_buffer;
  int len_received =
      uart_read_bytes(uart_num, response_buffer.data(), response_buffer.size(),
                      MAX_RESPONSE_TIMEOUT);
  if (len_received != response_buffer.size()) {
    return ESP_ERR_INVALID_SIZE;
  }

  // Compare sync nibble
  if (write_buffer[0] != response_buffer[0]) {
    ESP_LOGE(TAG, "Sync nibble on read response does not match");
    return ESP_ERR_INVALID_RESPONSE;
  }

  // Make sure response master address is valid
  if (response_buffer[1] != 0xFF) {
    ESP_LOGE(TAG, "Master address on read response is not 0xFF");
    return ESP_ERR_INVALID_RESPONSE;
  }

  // Compare register address and read flag
  if (write_buffer[2] != response_buffer[2]) {
    ESP_LOGE(TAG, "Register addr and read flag do not match on read response");
    return ESP_ERR_INVALID_RESPONSE;
  }

  // Check if CRC is correct
  uint8_t original_crc = response_buffer.back();
  set_crc(response_buffer.data(), response_buffer.size());
  if (original_crc != response_buffer.back()) {
    ESP_LOGE(TAG,
             "Response is corrupted, crc does not align with rest of message");
    return ESP_ERR_INVALID_RESPONSE;
  }

  output_buffer = (static_cast<uint32_t>(response_buffer[3]) << 24) |
                  (static_cast<uint32_t>(response_buffer[4]) << 16) |
                  (static_cast<uint32_t>(response_buffer[5]) << 8) |
                  static_cast<uint32_t>(response_buffer[6]);
  return ESP_OK;
}
