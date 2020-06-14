# WizNotePlus v2.8.0-rc.1 更新日志

## 系统要求

为了修复 #117 和 #112 提到的问题，预编译的 WizNotePlus 软件包现在基于 Qt 5.14.2 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.13
* Windows 最低要求 Windows 7

详情可见 [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.14/supported-platforms.html) 。

## 新的特征

* 新增 `WizDocument::SaveToFolder` API。
* 调整 “关于 WizNote” 对话框内容。

## 问题修复

* 内部编辑器复制及删除操作导致的错误行为。#117、#112

## 已知问题

- Windows 平台外部编辑器 Typora 保存时，有时会出现 WizNote 不同步更新的状况。#88
- MacOS 下“显示系统托盘图标”设置会导致程序崩溃。#76