#include "RedGuardScan.h"
#if defined(MARAUDER_CYD_2USB)
#include <string.h>
#include <Arduino.h>
namespace RedGuard {
static_assert(WIFI_AUTH_OPEN == 0 && WIFI_AUTH_WEP == 1 && WIFI_AUTH_WPA_PSK == 2 &&
 WIFI_AUTH_WPA2_PSK == 3 && WIFI_AUTH_WPA_WPA2_PSK == 4 && WIFI_AUTH_WPA2_ENTERPRISE == 5 &&
 WIFI_AUTH_WPA3_PSK == 6 && WIFI_AUTH_WPA2_WPA3_PSK == 7 && WIFI_AUTH_OWE == 9 &&
 WIFI_AUTH_WPA3_ENT_192 == 10 && WIFI_AUTH_WPA3_ENTERPRISE == 14 &&
 WIFI_AUTH_WPA2_WPA3_ENTERPRISE == 15 && WIFI_AUTH_WPA_ENTERPRISE == 16, "Review risk auth mapping");
static_assert(WIFI_CIPHER_TYPE_NONE == 0 && WIFI_CIPHER_TYPE_WEP40 == 1 &&
 WIFI_CIPHER_TYPE_WEP104 == 2 && WIFI_CIPHER_TYPE_TKIP == 3 && WIFI_CIPHER_TYPE_CCMP == 4 &&
 WIFI_CIPHER_TYPE_TKIP_CCMP == 5 && WIFI_CIPHER_TYPE_GCMP == 8 && WIFI_CIPHER_TYPE_GCMP256 == 9,
 "Review risk cipher mapping");
namespace {
// Persistent storage avoids a callback retaining a Scanner pointer during teardown.
PmfEvidence evidence;
portMUX_TYPE evidenceMux = portMUX_INITIALIZER_UNLOCKED;
bool captureActive = false;
uint32_t captureGeneration = 0;
void freezePmf() {
  portENTER_CRITICAL(&evidenceMux);
  captureActive = false;
  portEXIT_CRITICAL(&evidenceMux);
}
void beginPmf() {
  portENTER_CRITICAL(&evidenceMux);
  evidence.clear();
  ++captureGeneration;
  captureActive = true;
  portEXIT_CRITICAL(&evidenceMux);
}
Pmf observedPmf(const uint8_t bssid[6]) {
  portENTER_CRITICAL(&evidenceMux);
  const Pmf pmf = evidence.lookup(bssid);
  portEXIT_CRITICAL(&evidenceMux);
  return pmf;
}
void pmfCallback(void* buffer, wifi_promiscuous_pkt_type_t type) {
  if (!buffer || type != WIFI_PKT_MGMT) return;
  portENTER_CRITICAL(&evidenceMux);
  const bool active = captureActive;
  const uint32_t generation = captureGeneration;
  portEXIT_CRITICAL(&evidenceMux);
  if (!active) return;
  const auto* packet = static_cast<const wifi_promiscuous_pkt_t*>(buffer);
  const size_t length = packet->rx_ctrl.sig_len;
  PmfObservation observation;
  // The pinned ESP32 SDK defines sig_len as including four FCS bytes.
  if (parsePmfEsp32Packet(packet->payload,length,packet->rx_ctrl.rx_state == 0,observation) != AdvertisementStatus::Observed) return;
  portENTER_CRITICAL(&evidenceMux);
  if (captureActive && captureGeneration == generation) evidence.add(observation,packet->rx_ctrl.rssi);
  portEXIT_CRITICAL(&evidenceMux);
}
}
void Scanner::scanDone(void* arg, esp_event_base_t, int32_t, void* data) {
  auto* self = static_cast<Scanner*>(arg);
  const auto* event = static_cast<const wifi_event_sta_scan_done_t*>(data);
  self->status_.store(event ? event->status : 1, std::memory_order_relaxed);
  self->done_.store(true, std::memory_order_release);
}
void Scanner::releaseRadio() {
  if (handler_) {
    esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, handler_);
    handler_ = nullptr;
  }
  if (ownsDriver_) {
    freezePmf();
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(nullptr);
    esp_wifi_scan_stop();
    esp_wifi_clear_ap_list();
    esp_wifi_stop();
    esp_wifi_deinit();
    ownsDriver_ = false;
  }
  done_.store(false);
}
bool Scanner::start(uint32_t now) {
  if (state_ == ScanState::Running) return false;
  error_ = ESP_OK;
  // SDK 扫描也需要内部堆空间；快照有上限并不意味着 SDK 零分配。
  if (ESP.getFreeHeap() < 40960) { error_ = ESP_ERR_NO_MEM; state_ = ScanState::Failed; return false; }
  esp_err_t err = esp_event_loop_create_default();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) { error_ = err; state_ = ScanState::Failed; return false; }
  wifi_mode_t existing;
  if (esp_wifi_get_mode(&existing) == ESP_OK) {
    error_ = ESP_ERR_INVALID_STATE; state_ = ScanState::Failed; return false;
  }
  wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
  err = esp_wifi_init(&init);
  if (err != ESP_OK) { error_ = err; state_ = ScanState::Failed; return false; }
  ownsDriver_ = true;
  done_.store(false);
  if ((err = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &Scanner::scanDone, this, &handler_)) == ESP_OK &&
      (err = esp_wifi_set_storage(WIFI_STORAGE_RAM)) == ESP_OK &&
      (err = esp_wifi_set_mode(WIFI_MODE_STA)) == ESP_OK &&
      (err = esp_wifi_start()) == ESP_OK) {
    wifi_promiscuous_filter_t filter{};
    filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
    beginPmf();
    if ((err = esp_wifi_set_promiscuous_filter(&filter)) != ESP_OK ||
        (err = esp_wifi_set_promiscuous_rx_cb(&pmfCallback)) != ESP_OK ||
        (err = esp_wifi_set_promiscuous(true)) != ESP_OK) {
      error_ = err; releaseRadio(); state_ = ScanState::Failed; return false;
    }
    wifi_scan_config_t scan{};
    scan.show_hidden = true;
    scan.scan_type = WIFI_SCAN_TYPE_PASSIVE;
    scan.scan_time.passive = 300;
    err = esp_wifi_scan_start(&scan, false);
  }
  if (err != ESP_OK) { error_ = err; releaseRadio(); state_ = ScanState::Failed; return false; }
  startedAt_ = now;
  state_ = ScanState::Running;
  return true;
}
void Scanner::stop() {
  if (state_ != ScanState::Running) return;
  releaseRadio();
  state_ = ScanState::Cancelled;
}
bool Scanner::poll(uint32_t now) {
  if (state_ != ScanState::Running) return false;
  if (!done_.load(std::memory_order_acquire)) {
    if (uint32_t(now - startedAt_) < 15000) return false;
    error_ = ESP_ERR_TIMEOUT; releaseRadio(); state_ = ScanState::Failed; return true;
  }
  if (status_.load() != 0) {
    error_ = ESP_FAIL; releaseRadio(); state_ = ScanState::Failed; return true;
  }
  freezePmf(); // Freeze metadata before merging with the completed SDK snapshot.
  uint16_t count = 0;
  esp_err_t err = esp_wifi_scan_get_ap_num(&count);
  if (err != ESP_OK) { error_ = err; releaseRadio(); state_ = ScanState::Failed; return true; }
  uint16_t kept = count < Snapshot::kCapacity ? count : Snapshot::kCapacity;
  // 固定缓冲接收 SDK 按 RSSI 排序的前 64 条；该 API 会释放整个驱动列表。
  if (kept && (err = esp_wifi_scan_get_ap_records(&kept, records_)) != ESP_OK) {
    error_ = err; releaseRadio(); state_ = ScanState::Failed; return true;
  }
  results_.clear();
  for (uint16_t i = 0; i < kept; ++i) {
    const auto& record = records_[i];
    Ap ap;
    memcpy(ap.ssid, record.ssid, 32); ap.ssid[32] = '\0';
    memcpy(ap.bssid, record.bssid, 6);
    ap.pmf = observedPmf(ap.bssid);
    ap.channel = record.primary; ap.rssi = record.rssi;
    ap.auth = uint8_t(record.authmode); ap.pairwise = uint8_t(record.pairwise_cipher); ap.group = uint8_t(record.group_cipher); ap.wps = record.wps;
    results_.add(ap);
  }
  total_ = count;
  scannedAt_ = now;
  releaseRadio();
  state_ = ScanState::Complete;
  return true;
}
const char* authName(uint8_t auth) {
  switch (auth) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-EAP";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
    case WIFI_AUTH_OWE: return "OWE";
    case WIFI_AUTH_WPA3_ENT_192: return "WPA3-EAP-192";
    case WIFI_AUTH_WPA3_ENTERPRISE: return "WPA3-EAP";
    case WIFI_AUTH_WPA2_WPA3_ENTERPRISE: return "WPA2/WPA3-EAP";
    case WIFI_AUTH_WPA_ENTERPRISE: return "WPA-EAP";
    default: return "UNKNOWN";
  }
}
void Scanner::exportCsv() const {
  Serial.println("REDGUARD_CSV_BEGIN");
  Serial.println("ssid,bssid,channel,rssi_dbm,auth,uptime_ms,pmf,pairwise_cipher_id,group_cipher_id,wps_advertised");
  for (size_t i = 0; i < results_.size(); ++i) {
    const auto& ap = *results_.at(i);
    Serial.print("\"'");
    // 导出为电子表格查看时，阻止名称触发公式；原始数据保留在内存快照中。
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(ap.ssid); *p; ++p) {
      if (*p == '"') Serial.print('"');
      if (*p < 32 || *p == 127) Serial.print('?'); else Serial.write(*p);
    }
    Serial.printf("\",%02x:%02x:%02x:%02x:%02x:%02x,%u,%d,%s,%lu,%s,%u,%u,%u\r\n",
      ap.bssid[0],ap.bssid[1],ap.bssid[2],ap.bssid[3],ap.bssid[4],ap.bssid[5],ap.channel,ap.rssi,
      authName(ap.auth),static_cast<unsigned long>(scannedAt_),pmfName(ap.pmf),ap.pairwise,ap.group,unsigned(ap.wps));
  }
  Serial.println("REDGUARD_CSV_END");
}
}
#endif
