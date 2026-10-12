#pragma once
#include <stdint.h>
#include <stddef.h>
namespace RedGuard {
struct Ap {
  char ssid[33]{};
  uint8_t bssid[6]{};
  uint8_t channel = 0;
  int8_t rssi = -127;
  uint8_t auth = 255;
  uint8_t pairwise = 255;
  uint8_t group = 255;
  bool wps = false;
};
class Snapshot {
 public:
  static constexpr size_t kCapacity = 64;
  void clear();
  void add(const Ap& ap);
  size_t size() const { return count_; }
  const Ap* at(size_t index) const { return index < count_ ? &aps_[index] : nullptr; }
  uint16_t channelCount(unsigned channel) const;
 private:
  Ap aps_[kCapacity]{};
  size_t count_ = 0;
};
int signalPercent(int rssi);
}
