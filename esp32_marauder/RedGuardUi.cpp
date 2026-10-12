#include "RedGuardUi.h"
#include "RedGuardRender.h"
#if defined(MARAUDER_CYD_2USB)
#include <stdio.h>
#include "WiFiScan.h"
extern Display display_obj;
extern WiFiScan wifi_scan_obj;
namespace RedGuard {
namespace {
constexpr uint16_t background = kBackground, panel = kPanel, accent = kAccent;
constexpr TextId homeItems[] = {TextId::WifiScan, TextId::Security, TextId::Password,
  TextId::Signal, TextId::Ble, TextId::Network, TextId::Report, TextId::Settings};
}

void Ui::label(const char* value, int x, int y, int width, uint16_t color, bool centered) {
  drawLabel(display_obj.tft, value, x, y, width, color, centered);
}

void Ui::button(Rect rect, TextId id, bool selected) {
  display_obj.tft.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 5, panel);
  if (selected) display_obj.tft.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 5, accent);
  label(text(id), rect.x + 4, rect.y + (rect.h - 24) / 2, rect.w - 8,
        selected ? accent : TFT_WHITE, true);
}

void Ui::draw() {
  if (page_ == Page::ScanHub || page_ == Page::Results || page_ == Page::Detail || page_ == Page::Channels) {
    drawScanPage(); return;
  }
  auto& tft = display_obj.tft;
  tft.setRotation(SCREEN_ORIENTATION);
  tft.fillScreen(background);
  TextId title = TextId::Title;
  if (page_ == Page::Settings) title = TextId::Settings;
  if (page_ == Page::Info) title = TextId::DeviceInfo;
  if (page_ == Page::Notice) title = topic_;
  if (page_ == Page::Legacy) title = TextId::Advanced;
  label(text(title), 8, 8, 224, accent, true);
  tft.drawFastHLine(10, 41, 220, accent);
  if (page_ == Page::Home) {
    for (unsigned row = 0; row < 4; ++row) button(rowRect(row), homeItems[itemIndex(homePage_, row)], selected_ == itemIndex(homePage_, row));
    label(text(TextId::Navigation), 10, 252, 190, TFT_LIGHTGREY);
    label(homePage_ == 0 ? "1/2" : "2/2", 204, 252, 30, TFT_LIGHTGREY);
    button(bottomRect, TextId::Advanced, selected_ == 8);
  } else if (page_ == Page::Settings) {
    button(rowRect(0), TextId::DeviceInfo, selected_ == 0);
    button(rowRect(1), TextId::DeviceSettings, selected_ == 1);
    button(bottomRect, TextId::Back, selected_ == 2);
  } else if (page_ == Page::Notice) {
    label(text(TextId::Pending), 12, 70, 216, TFT_YELLOW);
    label(text(TextId::NoScan), 12, 110, 216, TFT_LIGHTGREY);
    label(text(TextId::Later), 12, 150, 216, TFT_LIGHTGREY);
    button(bottomRect, TextId::Back, page_ == Page::Settings ? selected_ == 2 : page_ == Page::Legacy ? selected_ == 1 : true);
  } else if (page_ == Page::Legacy) {
    label(text(TextId::LegacyNotice), 12, 64, 216, TFT_LIGHTGREY);
    label(text(TextId::LegacyTouch), 12, 100, 216, TFT_LIGHTGREY);
    button(rowRect(3), TextId::OpenLegacy, selected_ == 0);
    button(bottomRect, TextId::Back, page_ == Page::Settings ? selected_ == 2 : page_ == Page::Legacy ? selected_ == 1 : true);
  } else {
    // 实时读取设备信息；此页不触发扫描、连接或伪造电量。
    char number[40];
    label(text(TextId::Firmware), 12, 54, 216, TFT_LIGHTGREY);
    label(kUiVersion, 12, 80, 216, TFT_WHITE);
    label(text(TextId::Flash), 12, 114, 216, TFT_LIGHTGREY);
    snprintf(number, sizeof(number), "%lu KiB", (unsigned long)(ESP.getFlashChipSize() / 1024));
    label(number, 12, 140, 216, TFT_WHITE);
    label(text(TextId::Heap), 12, 174, 216, TFT_LIGHTGREY);
    snprintf(number, sizeof(number), "%lu bytes", (unsigned long)ESP.getFreeHeap());
    label(number, 12, 200, 216, TFT_WHITE);
    label(text(TextId::Band), 12, 244, 216, TFT_LIGHTGREY);
    button(bottomRect, TextId::Back, page_ == Page::Settings ? selected_ == 2 : page_ == Page::Legacy ? selected_ == 1 : true);
  }
}

UiAction Ui::poll(uint32_t now, bool touchEnabled) {
  if (scanner_.poll(now)) {
    scanMessage_ = scanner_.state() == ScanState::Complete ? TextId::ScanComplete : TextId::ScanFailed;
    if (page_ == Page::ScanHub || page_ == Page::Results || page_ == Page::Channels) draw();
  }
  if (!touchEnabled) { navigation_.reset(); return UiAction::None; }
  uint16_t x = 0, y = 0;
  const bool pressed = display_obj.updateTouch(&x, &y);
  const int8_t held = display_obj.menuButton(&x, &y, pressed, true);
  const auto event = navigation_.update(held, pressed, now);
  if (event == NavigationEvent::None) return UiAction::None;
  const unsigned count = page_ == Page::Home ? 9 : page_ == Page::Settings ? 3 : page_ == Page::Legacy ? 2 : page_ == Page::ScanHub ? 5 : page_ == Page::Results ? resultCount() + 2 : 1;
  if (event == NavigationEvent::Up || event == NavigationEvent::Down) {
    selected_ = event == NavigationEvent::Up ? (selected_ + count - 1) % count : (selected_ + 1) % count;
    if (page_ == Page::Home) homePage_ = selected_ < 4 ? 0 : 1;
    draw();
    return UiAction::None;
  }
  if (page_ == Page::ScanHub || page_ == Page::Results || page_ == Page::Detail || page_ == Page::Channels) {
    activateScanItem(); draw(); return UiAction::None;
  }
  if (page_ == Page::Home) {
    homeSelected_ = selected_;
    if (selected_ == 8) page_ = Page::Legacy;
    else {
      topic_ = homeItems[selected_];
      page_ = topic_ == TextId::Settings ? Page::Settings :
        (topic_ == TextId::WifiScan || topic_ == TextId::Signal) ? Page::ScanHub : Page::Notice;
    }
    selected_ = 0;
  } else if (page_ == Page::Settings && selected_ < 2) {
    if (selected_ == 1) return UiAction::DeviceSettings;
    page_ = Page::Info;
    selected_ = 0;
  } else if (page_ == Page::Legacy && selected_ == 0) {
    return UiAction::Upstream;
  } else if (page_ == Page::Info) {
    page_ = Page::Settings;
    selected_ = 0;
  } else {
    page_ = Page::Home;
    selected_ = homeSelected_;
  }
  draw();
  return UiAction::None;
}
}
#endif
