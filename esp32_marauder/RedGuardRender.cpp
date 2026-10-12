#include "RedGuardRender.h"
#if defined(MARAUDER_CYD_2USB)
#include "RedGuardModel.h"
namespace RedGuard {
void drawLabel(TFT_eSPI& tft, const char* value, int x, int y, int width,
               uint16_t color, bool centered) {
  const int right = x + width;
  if (centered) {
    int size = 0;
    const char* cursor = value;
    while (*cursor) size += glyph(nextCodepoint(cursor)).advance;
    if (size < width) x += (width - size) / 2;
  }
  while (*value) {
    const auto& g = glyph(nextCodepoint(value));
    if (x + g.advance > right || x + 20 > tft.width() || y < 0 || y + 24 > tft.height()) break;
    tft.drawBitmap(x, y, g.bitmap, 20, 24, color);
    x += g.advance;
  }
}

void drawBootStage(TFT_eSPI& tft, TextId stage) {
  // 只标记当前初始化步骤；函数返回并不冒充硬件自检通过。
  tft.fillRect(10, 238, 220, 54, kBackground);
  drawLabel(tft, text(stage), 10, 240, 220, TFT_LIGHTGREY, true);
  drawLabel(tft, text(TextId::BootChecking), 10, 270, 220, TFT_DARKGREY, true);
}

void drawBoot(TFT_eSPI& tft) {
  tft.fillScreen(kBackground);
  // 原创折线盾牌和 Wi-Fi 标识；不使用视频、外部图片或阻塞动画。
  const int shield[][2] = {{120,48},{174,68},{170,124},{151,153},{120,174},{89,153},{70,124},{66,68},{120,48}};
  for (unsigned i = 1; i < sizeof(shield)/sizeof(shield[0]); ++i) {
    tft.drawLine(shield[i-1][0], shield[i-1][1], shield[i][0], shield[i][1], kAccent);
    tft.drawLine(shield[i-1][0], shield[i-1][1]+1, shield[i][0], shield[i][1]+1, kAccent);
  }
  const int wifi[][2] = {{85,97},{94,89},{106,84},{120,82},{134,84},{146,89},{155,97}};
  for (unsigned i = 1; i < sizeof(wifi)/sizeof(wifi[0]); ++i)
    tft.drawLine(wifi[i-1][0],wifi[i-1][1],wifi[i][0],wifi[i][1],TFT_WHITE);
  tft.drawLine(96,108,106,102,TFT_WHITE); tft.drawLine(106,102,120,99,TFT_WHITE);
  tft.drawLine(120,99,134,102,TFT_WHITE); tft.drawLine(134,102,144,108,TFT_WHITE);
  tft.drawLine(108,120,120,116,TFT_WHITE); tft.drawLine(120,116,132,120,TFT_WHITE);
  tft.fillCircle(120,132,3,kAccent);
  drawLabel(tft, "RedGuard WiFi Lab", 10, 184, 220, TFT_WHITE, true);
  drawLabel(tft, text(TextId::Title), 10, 211, 220, kAccent, true);
  drawBootStage(tft, TextId::BootStarting);
  // 不增加 delay：画面停留由真实初始化耗时决定。
}
}
#endif
