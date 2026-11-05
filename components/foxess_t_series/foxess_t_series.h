#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "protocol.h"
#include <cstdio>
#include <vector>

namespace esphome {
namespace foxess_t_series {
class FoxessTSeries : public Component, public uart::UARTDevice {
 public:
  void set_sensor(size_t index, sensor::Sensor *value) { sensors_[index] = value; }
  void set_inverter_mode(text_sensor::TextSensor *value) { mode_ = value; }
  void set_faults(text_sensor::TextSensor *value) { faults_ = value; }
  void set_connected(binary_sensor::BinarySensor *value) { connected_ = value; }
  void setup() override {
    buffer_.reserve(525);
    mode_->publish_state("Waiting for data");
    connected_->publish_state(false);
  }
  void loop() override {
    const uint32_t now = millis();
    if (online_ && uint32_t(now - last_frame_) > 300000) {
      online_ = false;
      connected_->publish_state(false);
      mode_->publish_state("Offline");
      sensors_[32]->publish_state(0);
      // Do not integrate the last daytime PV power through the night.
      for (size_t index : {size_t(1), size_t(4), size_t(6), size_t(8), size_t(10),
                           size_t(12), size_t(14), size_t(16), size_t(18), size_t(20),
                           size_t(22), size_t(28), size_t(29), size_t(30), size_t(31)})
        sensors_[index]->publish_state(0);
    }
    // Discard an incomplete packet before joining bytes from a later packet.
    if (!buffer_.empty() && uint32_t(now - last_byte_) > 2000) buffer_.clear();
    while (available()) {
      uint8_t byte;
      if (!read_byte(&byte)) break;
      last_byte_ = millis();
      buffer_.push_back(byte);
      process_buffer_();
    }
  }
 protected:
  void process_buffer_() {
    while (buffer_.size() >= 3) {
      if (buffer_[0] != 0x7e || buffer_[1] != 0x7e || buffer_[2] != 0x02) {
        buffer_.erase(buffer_.begin());
        continue;
      }
      if (buffer_.size() < 9) return;
      const size_t payload = read_u16(buffer_.data() + 7);
      if (payload < 148 || payload > 512) {
        buffer_.erase(buffer_.begin());
        continue;
      }
      const size_t size = payload + 13;
      if (buffer_.size() < size) return;
      Reading reading;
      if (!decode_frame(buffer_.data(), size, reading)) {
        buffer_.erase(buffer_.begin());
        continue;
      }
      last_frame_ = millis();
      online_ = true;
      for (size_t i = 0; i < SENSOR_COUNT; i++) sensors_[i]->publish_state(reading.values[i]);
      connected_->publish_state(true);
      mode_->publish_state(reading.has_fault ? "Error" : "Online");
      std::string errors;
      char word[24];
      for (size_t i = 0; i < reading.faults.size(); i++) {
        if (!reading.faults[i]) continue;
        snprintf(word, sizeof(word), "%u:0x%08lX", unsigned(i + 1),
                 static_cast<unsigned long>(reading.faults[i]));
        if (!errors.empty()) errors += " ";
        errors += word;
      }
      faults_->publish_state(errors.empty() ? "None" : errors);
      buffer_.erase(buffer_.begin(), buffer_.begin() + size);
    }
  }
  std::array<sensor::Sensor *, SENSOR_COUNT> sensors_{};
  text_sensor::TextSensor *mode_{nullptr};
  text_sensor::TextSensor *faults_{nullptr};
  binary_sensor::BinarySensor *connected_{nullptr};
  std::vector<uint8_t> buffer_;
  uint32_t last_frame_{0}, last_byte_{0};
  bool online_{false};
};
}  // namespace foxess_t_series
}  // namespace esphome
