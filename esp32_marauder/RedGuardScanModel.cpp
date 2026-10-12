#include "RedGuardScanModel.h"
#include <string.h>
namespace RedGuard {
void Snapshot::clear() { count_ = 0; }
void Snapshot::add(const Ap& ap) {
  if (ap.channel < 1 || ap.channel > 14) return;
  size_t target = count_;
  for (size_t i = 0; i < count_; ++i) {
    if (!memcmp(aps_[i].bssid, ap.bssid, 6)) { target = i; break; }
  }
  if (target == count_) {
    if (count_ == kCapacity) {
      if (ap.rssi <= aps_[count_-1].rssi) return;
      target = count_-1;
    } else ++count_;
  }
  aps_[target] = ap;
  aps_[target].ssid[32] = '\0';
  // 固定容量插入排序；同信号按 BSSID 排序，结果稳定。
  for (size_t i = 1; i < count_; ++i) {
    Ap value = aps_[i];
    size_t j = i;
    while (j && (aps_[j-1].rssi < value.rssi ||
           (aps_[j-1].rssi == value.rssi && memcmp(aps_[j-1].bssid, value.bssid, 6) > 0))) {
      aps_[j] = aps_[j-1]; --j;
    }
    aps_[j] = value;
  }
}
uint16_t Snapshot::channelCount(unsigned channel) const {
  uint16_t result = 0;
  for (size_t i = 0; i < count_; ++i) if (aps_[i].channel == channel) ++result;
  return result;
}
int signalPercent(int rssi) { return rssi <= -100 ? 0 : rssi >= -50 ? 100 : (rssi + 100) * 2; }
}
