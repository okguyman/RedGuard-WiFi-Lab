#pragma once
#include <stddef.h>
#include <stdint.h>
namespace RedGuard {
enum class Pmf : uint8_t { Unknown, CapabilitiesMissing, NotAdvertised, Capable, Required, NoRsn, Invalid, Conflicting };
enum class AdvertisementStatus : uint8_t { Ignored, Malformed, Observed };
struct PmfObservation {
  uint8_t bssid[6]{};
  Pmf pmf = Pmf::Unknown;
};
// Input is a complete 802.11 beacon/probe-response, WITHOUT the trailing FCS.
AdvertisementStatus parsePmfAdvertisement(const uint8_t* frame, size_t length, PmfObservation& output);
AdvertisementStatus parsePmfEsp32Packet(const uint8_t* frame, size_t sigLength, bool rxOk, PmfObservation& output);
class PmfEvidence {
 public:
  static constexpr size_t kCapacity = 64;
  void clear() { count_ = 0; }
  void add(const PmfObservation& observation, int8_t rssi);
  Pmf lookup(const uint8_t bssid[6]) const;
  size_t size() const { return count_; }
 private:
  struct Entry { PmfObservation observation; int8_t rssi; };
  Entry entries_[kCapacity]{};
  size_t count_ = 0;
};
const char* pmfName(Pmf pmf);
}
