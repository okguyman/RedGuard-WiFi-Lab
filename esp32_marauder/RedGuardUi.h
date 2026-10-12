#pragma once
#include "configs.h"
#if defined(MARAUDER_CYD_2USB)
#include "Display.h"
#include "RedGuardModel.h"
#include "RedGuardText.h"
#include "RedGuardScan.h"

namespace RedGuard {
enum class UiAction : uint8_t { None, Upstream, DeviceSettings };
class Ui {
 public:
  void draw();
  void stopScan() {
    if (scanner_.state() == ScanState::Running) {
      scanner_.stop(); scanMessage_ = TextId::ScanCancelled;
      if (page_ == Page::ScanHub) draw();
    }
  }
  void show() { navigation_.reset(); draw(); }
  // 返回原版入口动作时由原有菜单系统接管；本页不会启动无线任务。
  UiAction poll(uint32_t now, bool touchEnabled);
 private:
  enum class Page : uint8_t { Home, Settings, Notice, Info, Legacy, ScanHub, Results, Detail, Channels, Risk, Password };
  Page page_ = Page::Home;
  unsigned homePage_ = 0;
  TextId topic_ = TextId::Title;
  Navigation navigation_;
  Scanner scanner_;
  unsigned detailIndex_ = 0;
  bool riskMode_ = false;
  bool riskAdvice_ = false;
  uint8_t channelFilter_ = 0;
  TextId scanMessage_ = TextId::NoResults;
  unsigned resultCount() const;
  const Ap* resultAt(unsigned index) const;
  void drawScanPage();
  void activateScanItem();
  unsigned selected_ = 0;
  unsigned homeSelected_ = 0;
  void label(const char* value, int x, int y, int width, uint16_t color, bool centered = false);
  void button(Rect rect, TextId id, bool selected = false);
};
}
#endif
