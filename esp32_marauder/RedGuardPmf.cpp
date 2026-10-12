#include "RedGuardPmf.h"
#include <string.h>
namespace RedGuard {
namespace {
uint16_t le16(const uint8_t* p) { return uint16_t(p[0]) | uint16_t(p[1]) << 8; }
bool suiteList(const uint8_t* data, size_t length, size_t& offset, size_t width, bool emptyAllowed) {
  if (length - offset < 2) return false;
  const unsigned count = le16(data + offset);
  offset += 2;
  if ((!count && !emptyAllowed) || count > (length - offset) / width) return false;
  offset += size_t(count) * width;
  return true;
}
bool rsnCapabilities(const uint8_t* data, size_t length, Pmf& pmf) {
  // Require explicit version, group cipher, pairwise list and AKM list.
  // Abbreviated/defaulted RSNEs remain unknown rather than inferring support.
  if (length < 6 || le16(data) != 1) return false;
  size_t offset = 6;
  if (!suiteList(data,length,offset,4,false) || !suiteList(data,length,offset,4,false)) return false;
  if (offset == length) { pmf = Pmf::CapabilitiesMissing; return true; }
  if (length - offset < 2) return false;
  const uint16_t capabilities = le16(data + offset);
  offset += 2;
  // Optional PMKID list is skipped, never retained. Optional management cipher is 4 bytes.
  if (offset != length) {
    if (!suiteList(data,length,offset,16,true)) return false;
    if (offset != length && length - offset != 4) return false;
  }
  const bool required = capabilities & (1u << 6), capable = capabilities & (1u << 7);
  pmf = required && !capable ? Pmf::Invalid : required ? Pmf::Required :
    capable ? Pmf::Capable : Pmf::NotAdvertised;
  return true;
}
}
AdvertisementStatus parsePmfAdvertisement(const uint8_t* frame, size_t length, PmfObservation& output) {
  output = PmfObservation{};
  if (!frame || length < 2 || length > 4091) return AdvertisementStatus::Ignored;
  if (frame[0] != 0x80 && frame[0] != 0x50) return AdvertisementStatus::Ignored;
  // Reject DS/protected/HT-control/fragmented frames instead of assuming a 24-byte header.
  if ((frame[1] & 0xc7) || length < 36 || (frame[22] & 0x0f)) return AdvertisementStatus::Malformed;
  const uint8_t* bssid = frame + 16;
  uint8_t nonzero = 0;
  for (unsigned i = 0; i < 6; ++i) nonzero |= bssid[i];
  if (!nonzero || (bssid[0] & 1) || memcmp(frame+10,bssid,6)) return AdvertisementStatus::Malformed;
  bool found = false;
  Pmf pmf = Pmf::NoRsn;
  size_t offset = 36;
  while (offset < length) {
    if (length - offset < 2) return AdvertisementStatus::Malformed;
    const uint8_t id = frame[offset], size = frame[offset+1];
    offset += 2;
    if (size > length - offset) return AdvertisementStatus::Malformed;
    if (id == 48) {
      if (found || !rsnCapabilities(frame+offset,size,pmf)) return AdvertisementStatus::Malformed;
      found = true;
    }
    offset += size;
  }
  memcpy(output.bssid,bssid,6);
  output.pmf = pmf;
  return AdvertisementStatus::Observed;
}
AdvertisementStatus parsePmfEsp32Packet(const uint8_t* frame, size_t sigLength, bool rxOk, PmfObservation& output) {
  output = PmfObservation{};
  if (!rxOk || sigLength < 4 || sigLength > 4095) return AdvertisementStatus::Ignored;
  return parsePmfAdvertisement(frame,sigLength-4,output);
}
void PmfEvidence::add(const PmfObservation& observation, int8_t rssi) {
  size_t index = 0;
  for (; index < count_; ++index) if (!memcmp(entries_[index].observation.bssid,observation.bssid,6)) break;
  if (index < count_) {
    auto& entry = entries_[index];
    // A valid explicit observation may replace missing capabilities, never erase a conflict.
    const Pmf old = entry.observation.pmf, next = observation.pmf;
    if (old != Pmf::Conflicting) {
      const bool rsnPresenceConflict = (old == Pmf::NoRsn && next == Pmf::CapabilitiesMissing) ||
        (old == Pmf::CapabilitiesMissing && next == Pmf::NoRsn);
      if (rsnPresenceConflict) entry.observation.pmf = Pmf::Conflicting;
      else if (old == Pmf::Unknown || old == Pmf::CapabilitiesMissing) {
        if (next != Pmf::Unknown) entry.observation.pmf = next;
      } else if (next != Pmf::Unknown && next != Pmf::CapabilitiesMissing && old != next)
        entry.observation.pmf = Pmf::Conflicting;
    }
    if (rssi > entry.rssi) entry.rssi = rssi;
    return;
  }
  if (count_ == kCapacity) {
    index = 0;
    for (size_t i = 1; i < count_; ++i) if (entries_[i].rssi < entries_[index].rssi) index = i;
    if (rssi <= entries_[index].rssi) return;
  } else ++count_;
  entries_[index] = Entry{observation,rssi};
}
Pmf PmfEvidence::lookup(const uint8_t bssid[6]) const {
  for (size_t i = 0; i < count_; ++i)
    if (!memcmp(entries_[i].observation.bssid,bssid,6)) return entries_[i].observation.pmf;
  return Pmf::Unknown;
}
const char* pmfName(Pmf pmf) {
  switch (pmf) {
    case Pmf::CapabilitiesMissing: return "capabilities_missing";
    case Pmf::NotAdvertised: return "not_advertised";
    case Pmf::Capable: return "capable";
    case Pmf::Required: return "required";
    case Pmf::NoRsn: return "no_rsn_observed";
    case Pmf::Invalid: return "invalid_claim";
    case Pmf::Conflicting: return "conflicting_claims";
    default: return "unknown";
  }
}
}
