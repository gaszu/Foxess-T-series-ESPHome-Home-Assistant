#include "../components/foxess_t_series/protocol.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
using namespace esphome::foxess_t_series;
int main() {
  std::vector<uint8_t> frame(165, 0);
  frame[0] = frame[1] = 0x7e; frame[2] = 2; frame[8] = 152;
  frame[163] = frame[164] = 0xe7;
  auto u16 = [&](size_t pos, uint16_t v) { frame[pos] = v >> 8; frame[pos + 1] = v & 0xff; };
  u16(11, 10000);  // AC generation power, original layout
  u16(39, 6000); u16(41, 100);  // PV1: 600 V * 10 A
  u16(69, 123);  // 12.3 kWh via existing YAML scaling
  frame[71] = 0x80; frame[72] = 0x12; frame[73] = 0x34; frame[74] = 0x56;
  Reading r;
  assert(decode_frame(frame.data(), frame.size(), r));
  assert(r.values[1] == 10000 && r.values[26] == 123);
  assert(std::abs(r.values[28] - 6000) < 0.01);
  assert(r.values[27] == float(0x80123456u));
  assert(!r.has_fault && r.values[32] == 1);
  frame[125] = 0x80; frame[156] = 0x01;
  assert(decode_frame(frame.data(), frame.size(), r));
  assert(r.faults[0] == 0x80000000u && r.faults[7] == 1u);
  assert(r.has_fault && r.values[32] == 2);
  // PV multiplication must not overflow signed 32-bit promotion.
  u16(39, 65535); u16(41, 65535);
  assert(decode_frame(frame.data(), frame.size(), r));
  assert(std::isfinite(r.values[28]) && r.values[28] > 42000000);
  for (size_t size = 0; size < frame.size(); size++)
    assert(!decode_frame(frame.data(), size, r));
  frame[164] = 0;
  assert(!decode_frame(frame.data(), frame.size(), r));
  frame[164] = 0xe7; frame[8] = 0;
  assert(!decode_frame(frame.data(), frame.size(), r));
  frame[7] = 0xff; frame[8] = 0xff;
  assert(!decode_frame(frame.data(), frame.size(), r));
  std::cout << "Protocol: valid readings, energy, faults, overflow and malformed frames passed\n";
}
