# PMF 被动观测（0.5.1）

PMF 是管理帧保护。本页只解释热点广播的能力与要求，不测试去认证、不尝试连接，也不声称验证某个客户端实际协商结果。

## 显示含义

| 显示 | CSV `pmf` | 解释 |
| --- | --- | --- |
| 广播要求保护 | `required` | 明确 RSN 能力位 MFPC=1、MFPR=1 |
| 广播支持可选 | `capable` | MFPC=1、MFPR=0；仍允许不使用保护的兼容连接 |
| 未声明支持 | `not_advertised` | 两位均为 0；与“未捕获到证据”不同 |
| 能力字段缺失 | `capabilities_missing` | 收到显式 RSN 前缀，但没有能力字段，不推断默认值 |
| 未见 RSN 字段 | `no_rsn_observed` | 收到格式完整的 Beacon/Probe Response，但该帧没有顶层 RSN |
| 声明位异常 | `invalid_claim` | MFPR=1、MFPC=0；需要核查 |
| 声明有冲突 | `conflicting_claims` | 保留观测中同一 BSSID 出现不同明确状态或 RSN 有无冲突 |
| 未知 | `unknown` | 没有匹配的可解析观测，可能是丢包、容量限制或不支持的帧 |

热点详情显示 PMF 行；“Wi-Fi 安全检测”的第二页显示同一状态。可选或未声明支持时建议在客户端兼容的前提下要求 PMF。冲突可能来自配置变化、帧来源或其它原因，不直接认定为恶意热点或已证实漏洞。

## 解析与资源边界

- 固定 ESP32 SDK 的 `sig_len` 包含 4 字节 FCS，适配层检查范围和 `rx_state` 后移除 FCS；不把它当作信息元素。
- 仅接受版本 0 的非分片、未保护、没有 DS/HT-control 扩展的 Beacon/Probe Response，核对发送地址与单播 BSSID。长度上限由经典 ESP32 的 12 位接收长度限定。
- 逐项检查 TLV、RSN 版本、单播套件/AKM 计数与长度、能力位及可选尾部边界。重复 RSN、截断、异常计数或尾部格式异常时拒绝整帧，不留下半解析结论。
- 不解析完整 RSN/RSNXE、MBSSID 非发送配置、厂商 RSNE override 或其它新扩展；非支持/缺失证据保持未知。缺少显式前缀的缩略 RSNE 也不猜默认值。
- 观测表独立限制为 64 个 BSSID，根据已收帧的峰值 RSSI 保留，和 SDK 排序快照不保证一一覆盖；遗漏匹配项为未知。容量淘汰的历史不保留，不声称检查了所有广播冲突。
- 同一保留项的明确冲突在本次扫描内保持，不用随后的一帧把冲突清掉。缺少能力字段不会抹掉已经收到的明确位值。
- 回调使用持久的有界表与临界区，不保留 Scanner 指针，不分配、不输出、不画屏；解析在锁外。扫描代次与冻结标志防止处理中的旧回调写入新的扫描表。
- 完成后先冻结，再与 SDK 快照合并；停止、超时或初始化失败时禁用/注销回调并释放自有驱动。启用接收过滤失败会让扫描失败，保留上次完整结果；不静默声称 PMF 检测成功。
- 不保存原始帧、PMKID、客户端流量或密码。仅查看广播声明，不能证明声明真实或所有客户端受到保护。

## 验证与尚未完成

原生测试覆盖显式 PMF 状态、能力缺失、截断、重复 RSN、溢出计数、可选尾部、异常头、错误接收状态、FCS 适配、64 条容量、冲突保持与清空。截断输入贴着不可读保护页测试；另运行 20000 个随机输入及 20000 个有效帧突变输入，使用 UBSan。

这些测试不等于真实射频与 SDK 生命周期验证。新增固件尚未刷入设备。仍需使用本人受控路由器核对关闭/可选/要求 PMF、取消/超时、与原版 Wi-Fi/BLE 工具交接、低内存及长期稳定性。严格确认没有发出 Probe Request 等帧，需独立受控抓包设备观察。

依据：[ESP-IDF v5.5.1 接收结构定义](https://github.com/espressif/esp-idf/blob/v5.5.1/components/esp_wifi/include/local/esp_wifi_types_native.h)、[Espressif 接收回调 API](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32/api-reference/network/esp_wifi.html)、[hostap 官方 PMF 能力位定义](https://w1.fi/hostapd/devel/wpa__common_8h_source.html)。解析器为本项目编写，没有复制外部解析器代码或新增运行依赖。
