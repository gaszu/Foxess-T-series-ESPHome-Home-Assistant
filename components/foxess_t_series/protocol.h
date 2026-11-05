#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace esphome {
namespace foxess_t_series {
// Offsets intentionally match the original working component. These are byte
// offsets in proprietary telemetry, NOT Modbus register addresses.
constexpr size_t SENSOR_COUNT = 33;
struct Reading {
  std::array<float, SENSOR_COUNT> values{};
  std::array<uint32_t, 8> faults{};
  bool has_fault = false;
};
inline uint16_t read_u16(const uint8_t *p) {
  return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}
inline uint32_t read_u32(const uint8_t *p) {
  return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
         (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}
inline bool decode_frame(const uint8_t *frame, size_t size, Reading &out) {
  if (size < 161 || frame[0] != 0x7e || frame[1] != 0x7e || frame[2] != 0x02)
    return false;
  const size_t payload_size = read_u16(frame + 7);
  if (payload_size < 148 || payload_size > 512 || size != payload_size + 13 ||
      frame[size - 2] != 0xe7 || frame[size - 1] != 0xe7)
    return false;
  const uint8_t *data = frame + 9;
  Reading result;
  // Values retain the raw units used by the existing YAML filters.
  for (size_t i = 0; i < 15; i++) result.values[i] = read_u16(data + i * 2);
  for (size_t pv = 0; pv < 4; pv++) {
    const uint16_t voltage = read_u16(data + 30 + pv * 6);
    const uint16_t current = read_u16(data + 32 + pv * 6);
    result.values[15 + pv * 2] = voltage;
    result.values[16 + pv * 2] = current;
    // Cast before multiplication: do not overflow signed integer promotion.
    result.values[28 + pv] = float(voltage) * float(current) * 0.01f;
  }
  for (size_t i = 0; i < 3; i++) result.values[23 + i] = read_u16(data + 54 + i * 2);
  result.values[26] = read_u16(data + 60);
  result.values[27] = read_u32(data + 62);
  for (size_t i = 0; i < 8; i++) {
    result.faults[i] = read_u32(data + 116 + i * 4);
    result.has_fault |= result.faults[i] != 0;
  }
  result.values[32] = result.has_fault ? 2 : 1;
  out = result;
  return true;
}
}  // namespace foxess_t_series
}  // namespace esphome
