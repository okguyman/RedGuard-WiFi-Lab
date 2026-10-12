#pragma once
#include "configs.h"
#if defined(MARAUDER_CYD_2USB)
#include <TFT_eSPI.h>
#include "RedGuardText.h"
namespace RedGuard {
constexpr uint16_t kBackground = 0x0841;
constexpr uint16_t kPanel = 0x18e3;
constexpr uint16_t kAccent = 0xf982;
constexpr const char* kUiVersion = "RedGuard UI 0.5.0";
// 字形直接从 Flash 绘制，无整屏缓冲或临时 String。
void drawLabel(TFT_eSPI& tft, const char* value, int x, int y, int width,
               uint16_t color, bool centered = false);
void drawBoot(TFT_eSPI& tft);
void drawBootStage(TFT_eSPI& tft, TextId stage);
}
#endif
