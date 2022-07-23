# WizNotePlus v2.11.0 更新日志

## 系统要求

预编译的 WizNotePlus 软件包基于 Qt 5.14.2 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.13
* Windows 最低要求 Windows 7

## 新的特征

* 初步实现笔记批量导出功能（感谢 @PikachuHy），更多功能需求请在 Github Issues 中反馈。#182
* 新增 JavaScript 插件类型，增强 Wiz.Note.Outline 功能。
* 支持创建、阅读及编辑协作笔记。#222
* 新增内置的页面放大缩小组件，插件 Wiz.Note.Zoom 将退役。
* 更改页面滚动条样式。

## 问题修复

* 修复较旧版本 QWebEngine 在 GLIBC >= 2.34 上崩溃的问题。#224
* 修复群组笔记文件夹的排序问题。

## 已知问题

见 [Known issues](/dist/known-issues-zh-CN.md) 文档。
