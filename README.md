# WizNotePlus

forked from [WizTeam/WizQTClient](https://github.com/WizTeam/WizQTClient)

README **中文** | [English](documents/README-en.md)

**为知笔记+** 是一款基于云服务的跨平台个人知识与时间管理工具。

## 介绍

**为知笔记+** 项目致力于提高 [为知笔记跨平台客户端](https://github.com/WizTeam/WizQTClient) 的可用性，以达到或者超越 Windows 平台特有客户端的水平。这个目标要求 **为知笔记+** 实现对开发者友好的插件系统，并且促进氛围良好的第三方开发者社区的建成。本项目在遵循 GPLv3 协议的情况下由第三方开发者发起并维护，欢迎任何有意愿参与项目贡献的开发者或用户联系 altair_wei@outlook.com 。

[![release](https://img.shields.io/badge/release-v2.7.3-green.svg)](https://github.com/altairwei/WizNotePlus/releases) [![license](https://img.shields.io/badge/license-GPLv3-green.svg)](https://github.com/altairwei/WizNotePlus/blob/master/LICENSE)

## 特点

### 适配 Linux/Windows 高分屏

修复 Linux 客户端在高分屏下无法检测正确 DPI 的问题，另外对 Window 客户端在高分屏下做了简单的适配。

### 支持 Chrome 开发者工具

增加 `F12` 快捷键打开开发者工具，用以调整笔记格式以及未来用作 `JavaScript` 插件的调试器。

### 多标签浏览文档和网页

![tabViewer](documents/images/tabViewer.png?raw=true)

### 外部编辑器

在编辑按钮右侧新增了一个菜单，用于添加和使用外部编辑器。

![extenalEditor](documents/images/external_editor.png?raw=true)

## 下载

请前往本项目 [Releases](https://github.com/altairwei/WizNotePlus/releases) 页面，根据用户平台和需求选择下载项。

## 使用

### DMG

macOS 平台下请双击 `WizNote-mac.dmg` ，并在弹出窗口中拖动应用包安装到相应位置。

### AppImage

Linux 平台下为 `WizNote-linux-x86_64.AppImage` 添加可执行权限后即可运行。如果想要将 AppImage 整合入桌面系统，请参考项目维基 [AppImage 整合入桌面环境](https://github.com/altairwei/WizNotePlus/wiki/AppImage%E6%95%B4%E5%90%88%E5%85%A5%E6%A1%8C%E9%9D%A2%E7%8E%AF%E5%A2%83) 。

### 编译

如果预编译和打包的程序无法在特定平台正常运行，可以考虑从源码编译。具体编译步骤请参见项目维基 [客户端编译步骤](https://github.com/altairwei/WizNotePlus/wiki/%E5%AE%A2%E6%88%B7%E7%AB%AF%E7%BC%96%E8%AF%91%E6%AD%A5%E9%AA%A4) 。

## 计划

- [x] Devtools 开发者工具。
- [x] 多标签浏览
- [x] 外部编辑器功能
- [ ] JavaScript 插件系统
- [ ] 更换默认富文本编辑器
- [ ] 建立其他云服务系统
- [ ] 皮肤或主题系统

## 依赖

- Qt 5.11 (L-GPL v3)
- QuaZIP (L-GPL V2.1)
- Cryptopp (Boost Software License 1.0)
- gumbo-query (MIT License)
- gumbo-parser (Apache License 2.0)
- create-dmg (MIT License)
