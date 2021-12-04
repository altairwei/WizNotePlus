# WizNotePlus v2.10.0 更新日志

## 系统要求

预编译的 WizNotePlus 软件包基于 Qt 5.14.2 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.13
* Windows 最低要求 Windows 7

## 新的特征

* 使用 [framelesshelper v2.0](https://github.com/wangwenx190/framelesshelper) 作为无边框窗口的实现方案。
* 使用 QSS 来定义窗口样式，采用了新的 UI 设计，“使用系统窗口样式” 设置不再可用。
* 为文档条目和文件夹条目添加了提示文本。
* 允许用户设置主窗口的界面字体。
* 应用程序更名为 WizNotePlus ，以避免与 WizNote X 的冲突。
* 欢迎页面新增 “最近阅读” 栏目。
* 内部富文本编辑器 “上标/下标” 功能。#188 by @notplus
* 提供环境变量 `WIZNOTE_DATA_STORE` 用于替换 `~/.wiznote`，以设置 WizNotePlus 数据文件夹位置。

## 问题修复

* 修复最后一个可见窗口的关闭引起应用程序退出的问题。#195

## 已知问题

* Windows 平台外部编辑器 Typora 保存时，有时会出现 WizNote 不同步更新的状况。#88
* MacOS 下“显示系统托盘图标”设置会导致程序崩溃。#76
* 笔记标注在不同平台的客户端上会有位置偏差。
* 单独笔记窗口在关闭时，偶尔会无法自动保存笔记内容。