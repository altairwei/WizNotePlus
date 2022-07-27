# WizNotePlus v2.10.1 更新日志

## 系统要求

预编译的 WizNotePlus 软件包基于 Qt 5.14.2 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.13
* Windows 最低要求 Windows 7

## 问题修复

* 将 CryptoPP 更新到 v8.5.0 版本，解决无法通过 Rosetta 2 在 Apple’s M1 Macs 上运行的问题。
* 修正 Welcome 页面笔记链接的下划线样式。
* 更新 framelesshelper 以修复鼠标在标题栏 hover 状态的失灵。wangwenx190/framelesshelper#92

## 已知问题

* Windows 平台外部编辑器 Typora 保存时，有时会出现 WizNote 不同步更新的状况。#88
* MacOS 下“显示系统托盘图标”设置会导致程序崩溃。#76
* 笔记标注在不同平台的客户端上会有位置偏差。
* 单独笔记窗口在关闭时，偶尔会无法自动保存笔记内容。