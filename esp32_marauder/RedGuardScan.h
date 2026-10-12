#pragma once
#include "configs.h"
#if defined(MARAUDER_CYD_2USB)
#include "RedGuardScanModel.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <atomic>
namespace RedGuard {
enum class ScanState : uint8_t { Idle, Running, Complete, Cancelled, Failed };
class Scanner {
 public:
  bool start(uint32_t now);
  void stop();
  bool poll(uint32_t now);
  const Snapshot& results() const { return results_; }
  ScanState state() const { return state_; }
  esp_err_t error() const { return error_; }
  uint16_t total() const { return total_; }
  uint32_t scannedAt() const { return scannedAt_; }
  // 只导出扫描结果，不包含任何用户输入的密码。
  void exportCsv() const;
 private:
  static void scanDone(void* arg, esp_event_base_t base, int32_t id, void* data);
  void releaseRadio();
  Snapshot results_;
  wifi_ap_record_t records_[Snapshot::kCapacity]{};
  ScanState state_ = ScanState::Idle;
  std::atomic<bool> done_{false};
  std::atomic<uint32_t> status_{0};
  esp_event_handler_instance_t handler_ = nullptr;
  bool ownsDriver_ = false;
  uint16_t total_ = 0;
  uint32_t startedAt_ = 0, scannedAt_ = 0;
  esp_err_t error_ = ESP_OK;
};
const char* authName(uint8_t auth);
}
#endif
