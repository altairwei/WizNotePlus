# WizNotePlus for Wins/MacOS/Linux

forked from [WizTeam/WizQTClient](https://github.com/WizTeam/WizQTClient)

## About WizQTClient

WizNote is an open-sourced project, published under GPLv3 for individual/personal users and custom commercial license for company users.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

The project is based on Qt, aimed to provide an excellent PKM(personal knowledge management) desktop environment based on cloud usage. At present, we only have Wiz cloud backend(our company) on the table. but we strong encourage developers to contribute to this project to add more cloud backend for different cloud providers like evernote, youdao, etc...even offline usage.

PKM should be an very important thing cross through one person's life, it's unwise to stick yourself to a fixed service provider or jump around and leave your collected info/secrets behide. PKM should be the same as your mind, fly over the ocean but never splash the waves.

freedom, means knowledge, means PKM, means this WizNote client.

if you are windows or portable platform users, we have WizNote for windows, ios, android from our [Homepage](http://www.wiznote.com)

## About WizNotePlus

New Wiznote.

## Compile (Windows, macOS, Linux)

Coming Soon !

## Feature

### 适配 Windows 端高分屏

OSX 端能提供系统本地对高分屏的支持，除了替换高分辨率图标外，不需要额外设置。linux 端对高分屏的支持很简单，只要在主进程之前设置 `QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);` ，就能完成对高分屏的支持。
相比之下，window端的高分屏适配就要复杂很多。

检查share/WizMisc.cpp中的 `WizLoadPixmapIcon` 函数，确保图标正确放大。

- [x] 笔记列表顶部工具栏两个按钮；
- [x] 目录树列表图标；
- [x] 目录树指示三角；通过Qt对高分屏的支持来解决
- [ ] 调整标题栏按钮布局，讲搜索组件变成固定宽度；
- [ ] 调整Windows设置来支持高分屏；
- [ ] 账户头像分辨率太低；

### 待实现特征

- [ ] 文档页面增加 Devtool
- [ ] 多标签页功能：使用定义好的QTabWidget，再将Wiz自己实现的Qt::FramelessWindowHint窗口嵌入到该组件中；
- [ ] 添加外置编辑器功能，用 QFileSystemWatcher 监控文件变动
- [ ] Windows 端插件添加 QAxObject 接口
- [ ] 建立插件系统
- [ ] 建立其他云服务系统
- [ ] 跟换内核为 Electron 内核

### 已知BUG

- [x] 非系统标题栏样式下切换笔记视图会导致软件崩溃
- [ ] 同步操作完成前无法正常退出
