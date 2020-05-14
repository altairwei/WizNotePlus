# WizNotePlus v2.8.0-beta.3

## 系统要求

预编译的 WizNotePlus 软件包现在基于 Qt 5.12.8 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.12
* Windows 最低要求 Windows 7

详情可见 [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.12/supported-platforms.html) 。

## 新的特征

* 新增 `WizCommonUI::Base64ToFile` API。
* 在 “关于 WizNote” 对话框中新增了更多的软件构建信息。

## 问题修复

* 菜单栏 “编辑” 项名称在中文翻译时错误地引入了全角右括号。
* Windows 平台客户端无法加载和使用笔记模板。
* 在笔记中插入或者粘贴图片时，因为图片文件的绝对地址没有自动被修改成相对地址，导致跨平台图片显示错误。#106

## 已知问题

- Windows 平台外部编辑器 Typora 保存时，有时会出现 WizNote 不同步更新的状况。#88
- MacOS 下“显示系统托盘图标”设置会导致程序崩溃。#76