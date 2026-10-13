#pragma once

#ifndef lang_var_h
#define lang_var_h


#include "configs.h"

//Starting window texts
PROGMEM const char text0_0[] = "正在准备串口…";
PROGMEM const char text0_1[] = "串口已启动";
PROGMEM const char text0_2[] = "内存检查完成";
PROGMEM const char text0_3[] = "SD卡已初始化";
PROGMEM const char text0_4[] = "SD卡初始化失败";
PROGMEM const char text0_5[] = "电池配置检查完成";
PROGMEM const char text0_6[] = "温度接口已初始化";
PROGMEM const char text0_7[] = "LED接口已初始化";
PROGMEM const char text0_8[] = "正在启动…";

//Single library (action) texts/Often used
PROGMEM const char text00[] = "电量已变化：";
PROGMEM const char text01[] = "文件已关闭";
PROGMEM const char text02[] = "无法打开文件 '";
PROGMEM const char text03[] = "开启";
PROGMEM const char text04[] = "关闭";
PROGMEM const char text05[] = "载入";
PROGMEM const char text06[] = "另存为";
PROGMEM const char text07[] = "退出";
PROGMEM const char text08[] = "设置";
PROGMEM const char text09[] = "返回";
PROGMEM const char text10[] = "信道：";
PROGMEM const char text11[] = "触摸屏幕退出";
PROGMEM const char text12[] = "取消";
PROGMEM const char text13[] = "保存";
PROGMEM const char text14[] = "是";
PROGMEM const char text15[] = "正在打开 /update.bin…";
PROGMEM const char text16[] = "关闭";
PROGMEM const char text17[] = "失败";
PROGMEM const char text18[] = "数据包/秒：";


//Menufunctions.cpp texts
PROGMEM const char text1_0[] = "SSID列表";
PROGMEM const char text1_1[] = "添加SSID";
PROGMEM const char text1_2[] = "SSID: ";
PROGMEM const char text1_3[] = "密码：";
PROGMEM const char text1_4[] = "设置已关闭";
PROGMEM const char text1_5[] = "设置已开启";
PROGMEM const char text1_6[] = "ESP32 Marauder ";
PROGMEM const char text1_7[] = "WiFi ";
PROGMEM const char text1_8[] = "Bad USB ";
PROGMEM const char text1_9[] = "设备 ";
PROGMEM const char text1_10[] = "通用工具 ";
PROGMEM const char text1_11[] = "正在更新… ";
PROGMEM const char text1_12[] = "选择方式 ";
PROGMEM const char text1_13[] = "确认更新 ";
PROGMEM const char text1_14[] = "ESP8266更新 ";
PROGMEM const char text1_15[] = "固件更新 ";
PROGMEM const char text1_16[] = "语言 ";
PROGMEM const char text1_17[] = "设备信息 ";
PROGMEM const char text1_18[] = "设置 ";
PROGMEM const char text1_19[] = "蓝牙 ";
PROGMEM const char text1_20[] = "Wi-Fi侦听 ";
PROGMEM const char text1_21[] = "Wi-Fi攻击 ";
PROGMEM const char text1_22[] = "Wi-Fi通用 ";
PROGMEM const char text1_23[] = "蓝牙侦听 ";
PROGMEM const char text1_24[] = "蓝牙通用 ";
PROGMEM const char text1_25[] = "关闭Wi-Fi ";
PROGMEM const char text1_26[] = "关闭BLE ";
PROGMEM const char text1_27[] = "生成SSID ";
PROGMEM const char text1_28[] = "清空SSID ";
PROGMEM const char text1_29[] = "清空热点 ";
PROGMEM const char text1_30[] = "重启";
PROGMEM const char text1_31[] = "侦听";
PROGMEM const char text1_32[] = "攻击";
PROGMEM const char text1_33[] = "通用";
PROGMEM const char text1_34[] = "蓝牙侦听";
PROGMEM const char text1_35[] = "检测盗刷设备";
PROGMEM const char text1_36[] = "BadUSB测试";
PROGMEM const char text1_37[] = "运行Ducky脚本";
PROGMEM const char text1_38[] = "绘图";
PROGMEM const char text1_39[] = "网页更新";
PROGMEM const char text1_40[] = "SD卡更新";
PROGMEM const char text1_41[] = "ESP8266更新";
PROGMEM const char text1_42[] = "探测请求侦听";
PROGMEM const char text1_43[] = "信标侦听";
PROGMEM const char text1_44[] = "断连帧侦听";
PROGMEM const char text1_45[] = "数据包监视";
PROGMEM const char text1_46[] = "EAPOL/PMKID扫描";
PROGMEM const char text1_47[] = "检测Pwnagotchi";
PROGMEM const char text1_48[] = "检测乐鑫设备";
PROGMEM const char text1_49[] = "扫描热点";
PROGMEM const char text1_50[] = "列表信标发送";
PROGMEM const char text1_51[] = "随机信标发送";
PROGMEM const char text1_52[] = "RickRoll信标";
PROGMEM const char text1_53[] = "探测请求洪泛";
PROGMEM const char text1_54[] = "断连帧洪泛";
PROGMEM const char text1_55[] = "连接Wi-Fi";
PROGMEM const char text1_56[] = "选择热点";
PROGMEM const char text1_57[] = "热点克隆发送";
PROGMEM const char text1_58[] = "原始抓包";
PROGMEM const char text1_59[] = "终端侦听";
PROGMEM const char text1_60[] = "清空终端";
PROGMEM const char text1_61[] = "选择终端";
PROGMEM const char text1_62[] = "定向断连";
PROGMEM const char text1_63[] = "检测Pineapple";
PROGMEM const char text1_64[] = "检测多SSID";
PROGMEM const char text1_65[] = "选择探测SSID";
PROGMEM const char text1_66[] = "GPS";  // Text label for GPS Menu in Main Menu
PROGMEM const char text1_67[] = "趣味SSID信标";


//SDInterface.cpp texts
PROGMEM const char text2_0[] = "找不到update.bin";
PROGMEM const char text2_1[] = "正在从SD卡更新…";
PROGMEM const char text2_2[] = "update.bin为空";
PROGMEM const char text2_3[] = "\n正在重启…\n";
PROGMEM const char text2_4[] = "无法从根目录载入update.bin";
PROGMEM const char text2_5[] = "文件大小：";
PROGMEM const char text2_6[] = "正在写入分区…";
PROGMEM const char text2_7[] = "已写入：";
PROGMEM const char text2_8[] = "仅写入：";
PROGMEM const char text2_9[] = "，重试？";
PROGMEM const char text2_10[] = " 成功";
PROGMEM const char text2_11[] = "更新完成";
PROGMEM const char text2_12[] = "更新未能完成";
PROGMEM const char text2_13[] = "发生错误，编号：";
PROGMEM const char text2_14[] = "空间不足，无法开始OTA";

//Web.cpp texts
PROGMEM const char text3_0[] = "正在配置更新服务器…\n\n";
PROGMEM const char text3_1[] = "IP地址：";
PROGMEM const char text3_2[] = "更新：";
PROGMEM const char text3_3[] = "已完成字节数：";
PROGMEM const char text3_4[] = "更新成功：";
PROGMEM const char text3_5[] = "\n更新服务器设置完成";

//WiFiScan.cpp texts
PROGMEM const char text4_0[] = " RSSI: ";
PROGMEM const char text4_1[] = "疑似盗刷设备：";
PROGMEM const char text4_2[] = "已经连接";
PROGMEM const char text4_3[] = "连接失败";
PROGMEM const char text4_4[] = "已连接";
PROGMEM const char text4_5[] = "ForcePMKID";
PROGMEM const char text4_6[] = "ForceProbe";
PROGMEM const char text4_7[] = "SavePCAP";
PROGMEM const char text4_8[] = "探测洪泛";
PROGMEM const char text4_9[] = "正在清空热点…";
PROGMEM const char text4_10[] = "已清空热点：";
PROGMEM const char text4_11[] = "正在清空SSID…";
PROGMEM const char text4_12[] = "已清空SSID：";
PROGMEM const char text4_13[] = "正在生成SSID…";
PROGMEM const char text4_14[] = "已生成SSID：";        //Add spaces before to match : [15]
PROGMEM const char text4_15[] = "    SSID总数：";        //Add spaces beforer to match : [14]
PROGMEM const char text4_16[] = "正在关闭Wi-Fi…";
PROGMEM const char text4_17[] = "Wi-Fi尚未初始化";
PROGMEM const char text4_18[] = "正在关闭BLE…";
PROGMEM const char text4_19[] = "BLE尚未初始化";
PROGMEM const char text4_20[] = "固件：Marauder";      //From 20 to 35 add spaces so : is in line like it is now
PROGMEM const char text4_21[] = "版本：";
PROGMEM const char text4_22[] = "ESP-IDF: ";
PROGMEM const char text4_23[] = "WSL绕过：开启";
PROGMEM const char text4_24[] = "WSL绕过：关闭";
PROGMEM const char text4_25[] = "终端MAC：";
PROGMEM const char text4_26[] = "热点MAC：";
PROGMEM const char text4_27[] = "";
PROGMEM const char text4_28[] = "SD卡：已连接";
PROGMEM const char text4_29[] = "SD卡容量：";
PROGMEM const char text4_30[] = "SD卡：未连接";
PROGMEM const char text4_31[] = "SD卡容量：0";
PROGMEM const char text4_32[] = "电量检测：支持";
PROGMEM const char text4_33[] = "电量：";
PROGMEM const char text4_34[] = "电量检测：不支持";
PROGMEM const char text4_35[] = "内部温度：";
PROGMEM const char text4_36[] = " 检测乐鑫设备 ";
PROGMEM const char text4_37[] = " 检测Pwnagotchi ";
PROGMEM const char text4_38[] = " 信标侦听 ";
PROGMEM const char text4_39[] = " 断连帧侦听 ";
PROGMEM const char text4_40[] = " 探测请求侦听 ";
PROGMEM const char text4_41[] = " 蓝牙侦听 ";
PROGMEM const char text4_42[] = " 检测盗刷设备 ";
PROGMEM const char text4_43[] = "正在扫描蓝牙盗刷设备\nHC-03、HC-05、HC-06…";
PROGMEM const char text4_44[] = " 热点扫描 ";
PROGMEM const char text4_45[] = "正在清空终端…";
PROGMEM const char text4_46[] = "已清空终端：";
PROGMEM const char text4_47[] = "定向断连";
PROGMEM const char text4_48[] = " 检测Pineapple ";
PROGMEM const char text4_49[] = " 检测多SSID ";

//Making tables
static PROGMEM const char *text_table0[] = {text0_0,text0_1, text0_2, text0_3, text0_4, text0_5, text0_6, text0_7, text0_8};
static PROGMEM const char *text_table1[] = {text1_0,text1_1,text1_2,text1_3,text1_4,text1_5,text1_6,text1_7,text1_8,text1_9,text1_10,text1_11,text1_12,text1_13,text1_14,text1_15,text1_16,text1_17,text1_18,text1_19,text1_20,text1_21,text1_22,text1_23,text1_24,text1_25,text1_26,text1_27,text1_28,text1_29,text1_30,text1_31,text1_32,text1_33,text1_34,text1_35,text1_36,text1_37,text1_38,text1_39,text1_40,text1_41,text1_42,text1_43,text1_44,text1_45,text1_46,text1_47,text1_48,text1_49,text1_50,text1_51,text1_52,text1_53,text1_54,text1_55,text1_56,text1_57,text1_58,text1_59,text1_60,text1_61,text1_62,text1_63,text1_64, text1_65, text1_66, text1_67};
static PROGMEM const char *text_table2[] = {text2_0,text2_1,text2_2,text2_3,text2_4,text2_5,text2_6,text2_7,text2_8,text2_9,text2_10,text2_11,text2_12,text2_13,text2_14};
static PROGMEM const char *text_table3[] = {text3_0,text3_1,text3_2,text3_3,text3_4,text3_5};
static PROGMEM const char *text_table4[] = {text4_0,text4_1,text4_2,text4_3,text4_4,text4_5,text4_6,text4_7,text1_54,text4_9,text4_10,text4_11,text4_12,text4_13,text4_14,text4_15,text4_16,text4_17,text4_18,text4_19,text4_20,text4_21,text4_22,text4_23,text4_24,text4_25,text4_26,text4_27,text4_28,text4_29,text4_30,text4_31,text4_32,text4_33,text4_34,text4_35,text4_36,text4_37,text4_38,text4_39,text4_40,text4_41,text4_42,text4_43,text4_44,text4_45,text4_46,text4_47,text4_48,text4_49};

#endif
