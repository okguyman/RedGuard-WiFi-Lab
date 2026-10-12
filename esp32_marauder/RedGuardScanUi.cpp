#include "RedGuardUi.h"
#if defined(MARAUDER_CYD_2USB)
#include "RedGuardRender.h"
#include "RedGuardRisk.h"
#include "WiFiScan.h"
#include <stdio.h>
extern Display display_obj;
extern WiFiScan wifi_scan_obj;
namespace RedGuard {
unsigned Ui::resultCount() const {
  unsigned n = 0;
  for (unsigned i = 0; i < scanner_.results().size(); ++i)
    if (!channelFilter_ || scanner_.results().at(i)->channel == channelFilter_) ++n;
  return n;
}
const Ap* Ui::resultAt(unsigned index) const {
  for (unsigned i = 0; i < scanner_.results().size(); ++i) {
    const auto* ap = scanner_.results().at(i);
    if (!channelFilter_ || ap->channel == channelFilter_) {
      if (!index--) return ap;
    }
  }
  return nullptr;
}
void Ui::drawScanPage() {
  auto& tft = display_obj.tft;
  tft.setRotation(SCREEN_ORIENTATION);
  tft.fillScreen(kBackground);
  label(text(page_ == Page::Channels ? TextId::Channels : riskMode_ || page_ == Page::Risk ? TextId::Security : TextId::WifiScan), 8, 8, 224, kAccent, true);
  tft.drawFastHLine(10,41,220,kAccent);
  char line[64];
  const unsigned count = resultCount();
  if (page_ == Page::ScanHub) {
    button(rowRect(0), scanner_.state() == ScanState::Running ? TextId::StopScan : TextId::StartScan, selected_ == 0);
    button(rowRect(1), TextId::ViewResults, selected_ == 1);
    button(rowRect(2), TextId::Channels, selected_ == 2);
    button(rowRect(3), TextId::ExportCsv, selected_ == 3);
    label(text(scanMessage_), 10, 252, 220, TFT_LIGHTGREY);
    button(bottomRect, TextId::Back, selected_ == 4);
  } else if (page_ == Page::Results) {
    if (!count) label(text(TextId::NoResultHint),10,104,220,TFT_LIGHTGREY);
    const unsigned first = (selected_ / 4) * 4;
    for (unsigned row = 0; row < 4; ++row) {
      const unsigned index = first + row;
      if (index > count) break;
      if (index == count) {
        button(rowRect(row), TextId::Filter, selected_ == index);
        continue;
      }
      const auto* ap = resultAt(index);
      const auto rect = rowRect(row);
      tft.fillRoundRect(rect.x,rect.y,rect.w,rect.h,5,kPanel);
      if (selected_ == index) tft.drawRoundRect(rect.x,rect.y,rect.w,rect.h,5,kAccent);
      label(ap->ssid[0] ? ap->ssid : text(TextId::Hidden), 14, rect.y-1, 212, selected_ == index ? kAccent : TFT_WHITE);
      // 第二行采用小型内置字体，完整名称在详情页查看并可 USB 导出。
      snprintf(line,sizeof(line),"%d dBm   CH:%u   %s",ap->rssi,ap->channel,authName(ap->auth));
      tft.setFreeFont(nullptr); tft.setTextSize(1); tft.setTextColor(TFT_LIGHTGREY,kPanel);
      tft.drawString(line,14,rect.y+27,1);
    }
    snprintf(line,sizeof(line),"%u/%u  CH:%s",count,scanner_.total(),channelFilter_ ? "" : "ALL");
    if (channelFilter_) snprintf(line,sizeof(line),"%u/%u  CH:%u",count,scanner_.total(),channelFilter_);
    label(line,10,252,220,TFT_LIGHTGREY);
    button(bottomRect,TextId::Back,selected_ == count+1);
  } else if (page_ == Page::Risk) {
    const auto* ap = resultAt(detailIndex_);
    if (ap) {
      const auto risk = assess(*ap);
      label(ap->ssid[0] ? ap->ssid : text(TextId::Hidden),10,50,220,TFT_WHITE);
      if (!riskAdvice_) {
        label(text(risk.level),10,82,220,risk.level == TextId::RiskHigh ? TFT_YELLOW : TFT_WHITE);
        label(text(risk.authentication),10,114,220,TFT_LIGHTGREY);
        label(text(risk.cipher),10,146,220,TFT_LIGHTGREY);
        label(text(risk.wps),10,178,220,TFT_LIGHTGREY);
      } else {
        label(text(risk.advice),10,82,220,TFT_WHITE);
        label(text(TextId::UnknownPmf),10,114,220,TFT_LIGHTGREY);
        label(text(TextId::UnknownPassword),10,146,220,TFT_LIGHTGREY);
        label(text(TextId::UnknownRouter),10,178,220,TFT_LIGHTGREY);
      }
      button(Rect{10,212,220,34},TextId::NextPage,selected_ == 0);
      label(text(TextId::RiskLimit),10,252,220,TFT_LIGHTGREY);
    }
    button(bottomRect,TextId::Back,selected_ == 1);
  } else if (page_ == Page::Detail) {
    const auto* ap = resultAt(detailIndex_);
    if (ap) {
      label(ap->ssid[0] ? ap->ssid : text(TextId::Hidden),10,50,220,TFT_WHITE);
      snprintf(line,sizeof(line),"%02x:%02x:%02x:%02x:%02x:%02x",ap->bssid[0],ap->bssid[1],ap->bssid[2],ap->bssid[3],ap->bssid[4],ap->bssid[5]);
      label(line,10,78,220,TFT_LIGHTGREY);
      snprintf(line,sizeof(line),"CH:%u  %d dBm  %d%%",ap->channel,ap->rssi,signalPercent(ap->rssi));
      label(line,10,106,220,TFT_WHITE);
      label(authName(ap->auth),10,134,220,TFT_WHITE);
      label(text(TextId::UnknownPmf),10,162,220,TFT_LIGHTGREY);
      label(text(ap->wps ? TextId::WpsPresent : TextId::WpsUnobserved),10,190,220,TFT_LIGHTGREY);
      snprintf(line,sizeof(line),"+%lus",static_cast<unsigned long>(scanner_.scannedAt()/1000));
      tft.setFreeFont(nullptr);tft.setTextSize(1);tft.setTextColor(TFT_LIGHTGREY,kBackground);tft.drawString(line,10,230,1);
      tft.fillRect(10,216,220,8,kPanel);tft.fillRect(10,216,signalPercent(ap->rssi)*220/100,8,kAccent);
      label(text(TextId::SignalScale),10,242,220,TFT_LIGHTGREY);
    } else label(text(TextId::NoResults),10,80,220,TFT_LIGHTGREY);
    button(bottomRect,TextId::Back,true);
  } else {
    unsigned highest = 1;
    for (unsigned ch = 1; ch <= 14; ++ch) if (scanner_.results().channelCount(ch)>highest) highest=scanner_.results().channelCount(ch);
    for (unsigned ch = 1; ch <= 14; ++ch) {
      const int x = 10 + (ch-1)*16;
      const unsigned n = scanner_.results().channelCount(ch);
      const int height = n*116/highest;
      tft.fillRect(x,204-height,12,height,kAccent);
      tft.setFreeFont(nullptr);tft.setTextSize(1);tft.setTextColor(TFT_WHITE,kBackground);
      snprintf(line,sizeof(line),"%u",ch);tft.drawString(line,x,210,1);
      if (n) {snprintf(line,sizeof(line),"%u",n);tft.drawString(line,x,190-height,1);}
    }
    snprintf(line,sizeof(line),"AP:%u/%u  max:%u",unsigned(scanner_.results().size()),scanner_.total(),highest);
    tft.setFreeFont(nullptr);tft.setTextSize(1);tft.setTextColor(TFT_LIGHTGREY,kBackground);tft.drawString(line,10,54,1);
    label(text(TextId::CountOnly),10,244,220,TFT_LIGHTGREY);
    button(bottomRect,TextId::Back,true);
  }
}
void Ui::activateScanItem() {
  if (page_ == Page::ScanHub) {
    if (selected_ >= 1 && selected_ <= 3 && scanner_.state() == ScanState::Running) {
      scanMessage_ = TextId::Scanning; return;
    }
    if (selected_ == 0) {
      if (scanner_.state() == ScanState::Running) { scanner_.stop(); scanMessage_=TextId::ScanCancelled; }
      else if (wifi_scan_obj.currentScanMode != WIFI_SCAN_OFF || wifi_scan_obj.wifi_connected) scanMessage_=TextId::DisconnectFirst;
      else {
        wifi_scan_obj.shutdownWiFi();
        scanMessage_=scanner_.start(millis()) ? TextId::Scanning : TextId::ScanFailed;
      }
    } else if (selected_ == 1) { page_=Page::Results; selected_=0; }
    else if (selected_ == 2) { page_=Page::Channels; selected_=0; }
    else if (selected_ == 3) {
      scanner_.exportCsv();scanMessage_=TextId::CsvSent;
    } else {
      scanner_.stop();page_=Page::Home;selected_=homeSelected_;
    }
  } else if (page_ == Page::Results) {
    const unsigned count=resultCount();
    if (selected_ < count) { detailIndex_=selected_;page_=riskMode_ ? Page::Risk : Page::Detail;riskAdvice_=false;selected_=0; }
    else if (selected_ == count) {channelFilter_=(channelFilter_+1)%15;selected_=resultCount();}
    else {page_=riskMode_ ? Page::Home : Page::ScanHub;selected_=riskMode_ ? homeSelected_ : 1;}
  } else if (page_ == Page::Risk) {
    if (selected_ == 0) riskAdvice_=!riskAdvice_;
    else {page_=Page::Results;selected_=detailIndex_;}
  } else if (page_ == Page::Detail) {page_=Page::Results;selected_=detailIndex_;}
  else {page_=Page::ScanHub;selected_=2;}
}
}
#endif
