# ESP32 Marauder 中文界面（CYD 双 USB）

## English overview

Unofficial Chinese localization of ESP32 Marauder v1.18.0 for the ESP32-2432S028R dual-USB CYD board (ST7789, 4 MB flash), maintained by okguyman.

This downstream work covers Chinese menu text, a 414-glyph embedded Noto Sans SC font subset, scrolling boundaries, pinned build tooling, and hardware-specific installation notes. The underlying firmware functionality belongs to the upstream project and its contributors. This is not an official upstream release.

Repository records dated October 13, 2026 report working Chinese display and touch on one matching board. Wider hardware compatibility and complete functional coverage remain unverified. There are no verified external adoption figures yet.

Source provenance: [ESP32 Marauder](https://github.com/justcallmekoko/ESP32Marauder), revision `b2b9f92382eb60b39b8741be3b567e416cdbe672`. See `LICENSE`, `assets/zh/OFL.txt`, `tools/build_zh.py`, and `docs/zh/` for licensing, build tooling, and recorded validation. Use only in environments you own or are authorized to test.

## 中文说明

基于官方 v1.18.0，提交 b2b9f92382eb60b39b8741be3b567e416cdbe672。
上游：https://github.com/justcallmekoko/ESP32Marauder

适配本次已识别的 ESP32-2432S028R 双 USB、ST7789 屏幕、4MB Flash。其他屏幕版本不可直接套用。

保留原版菜单层级、功能、触控和配置；删除此前红盾自定义功能。修改限于中文界面、内置字体和中文滚动边界。设备名称、SSID、MAC、协议名及命令保持原样。字体 Noto Sans SC，许可见 assets/zh/OFL.txt。本构建是本地汉化版本，并非官方发布的中文二进制。

原版触控：按屏幕上部向上选择，下部向下选择，中间确认。返回使用菜单中的返回项。

源码 esp32_marauder/；中文词条 assets/zh/strings.json。使用 Arduino ESP32 3.3.4、TFT_eSPI 2.5.34、MARAUDER_CYD_2USB 和 min_spiffs。构建工具 tools/build_zh.py 只编译，不刷写。

本机当前分区的应用地址为 0x10000，容量 0x1e0000。此次只更新应用，保留已验证的引导程序及分区表。不要把此地址推广到不同分区的设备。

仅在自有或明确获准的无线环境测试。AP 扫描列出附近网络，不能直接读取 Wi-Fi 密码。

## 已验证结果

2026-10-13：应用、引导程序和分区表独立校验通过；启动后串口正常。用户确认中文显示与触摸正常。未逐项验证所有无线功能。

构建：`python3 tools/build_zh.py --setup`。已刷入的应用文件位于 `firmware/zh-CYD-2USB/esp32_marauder.ino.bin`，校验值见同目录 `SHA256SUMS`。该文件仅用于匹配本说明硬件及现有分区的应用更新，不是全片合并镜像。
