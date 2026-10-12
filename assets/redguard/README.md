# RedGuard 字体与中文资源

`text.json` 是新界面的统一中文资源。`RedGuardText.h/.cpp` 由 `tools/generate_redguard_font.py` 自动生成。

字体来源：Google Fonts 官方仓库 `ofl/notosanssc/NotoSansSC[wght].ttf`，SIL Open Font License 1.1，完整许可在 `OFL.txt`；生成字库属于修改后的嵌入式子集，不使用保留字体名称作为新字体名称。字体原件不打包进固件。源文件 SHA-256、字形数、位图尺寸和文本宽度记录在 `font-manifest.json`。官方源码地址：https://github.com/google/fonts/tree/main/ofl/notosanssc 。

生成方法（需要 Python/Pillow）：

```sh
python3 tools/generate_redguard_font.py /path/to/NotoSansSC.ttf
```

字库覆盖界面文字和可打印 ASCII，实际字形数见 font-manifest.json，18px、字重 500 单色、20×24 单元。动态名称的缺字显示 `?`；不宣称支持所有中文 SSID。本阶段未处理 Wi-Fi 列表的中文化。

触控沿用原版上移/中选/下移区域、抬手确认和连续移动规则；菜单每页四项，光标自动翻页。原版高级工具继续使用原有英文界面。
