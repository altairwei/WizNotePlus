# WizNotePlus for Wins/MacOS/Linux

forked from [WizTeam/WizQTClient](https://github.com/WizTeam/WizQTClient)

## About WizQTClient

### cross-platform cloud based note-taking client

WizNote is an open-sourced project, published under GPLv3 for individual/personal users and custom commercial license for company users.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

### Introduction to  WizQTClient

The project is based on Qt, aimed to provide an excellent PKM(personal knowledge management) desktop environment based on cloud usage. At present, we only have Wiz cloud backend(our company) on the table. but we strong encourage developers to contribute to this project to add more cloud backend for different cloud providers like evernote, youdao, etc...even offline usage.

PKM should be an very important thing cross through one person's life, it's unwise to stick yourself to a fixed service provider or jump around and leave your collected info/secrets behide. PKM should be the same as your mind, fly over the ocean but never splash the waves.

freedom, means knowledge, means PKM, means this WizNote client.

if you are windows or portable platform users, we have WizNote for windows, ios, android from our [Homepage](http://www.wiznote.com)

## About WizNotePlus

致力于开发新的开源版为知笔记客户端。

## Compile (Windows, macOS, Linux)

需要高于Qt5.11的版本才能编译成功。

## Feature

### 适配 Linux/Windows 高分屏

macOS 端能提供系统本地对高分屏的支持，除了替换高分辨率图标外，不需要额外设置。linux 端对高分屏的支持很简单，只要在主进程之前设置 `QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);` ，就能完成对高分屏的支持。相比之下，Window 端的高分屏适配就要复杂很多，目前采用了将字体回调到小字号的办法来暂时适配高分屏，这种做法的缺陷是字体稍有有一点儿模糊。Windows 端高分屏适配除了字体问题外，还有某些图标的显示上存在问题。

- [x] 笔记列表顶部工具栏两个按钮；
- [x] 目录树列表图标；
- [x] 目录树指示三角；通过Qt对高分屏的支持来解决
- [x] 调整标题栏按钮布局，将搜索组件变成固定宽度；
- [ ] 账户头像分辨率太低；

### Chrome 开发者工具

增加F1快捷键打开开发者工具，用以调整笔记格式以及未来用作JavaScript插件的调试器。

### 多标签浏览文档和网页

通过QTabWidget部件实现多标签浏览文档和网页的功能。

![tabViewer](documents/images/tabViewer.png?raw=true)

## Project Plan

- [x] 文档页面增加 Devtools
- [x] 多标签页功能：使用QTabWidget实现多标签，将WizDocumentView部件嵌入WizMainTab的Page中
- [x] 浏览网页功能：新建WizWebsiteView类，包裹WizWebEngineView部件并嵌入WizMainTab
- [ ] 添加外置编辑器功能，用 QFileSystemWatcher 监控文件变动
- [ ] Windows 端插件添加 QAxObject 接口
- [ ] 建立插件系统
- [ ] 建立其他云服务系统
- [ ] 更换内核为 Electron 内核

## 已知问题

- [x] 非系统标题栏样式下切换笔记视图会导致软件崩溃
- [ ] Windows 笔记渲染具有较严重的性能问题。
