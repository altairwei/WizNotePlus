# WizNotePlus for Linux/OSX/Windows

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

需要高于Qt5.11的版本才能编译成功。相关文档正在撰写中。

## Feature

### 适配 Linux/Windows 高分屏

macOS 端能提供系统本地对高分屏的支持，除了替换高分辨率图标外，不需要额外设置。linux 端对高分屏的支持很简单，只要在主进程之前设置 `QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);` ，就能完成对高分屏的支持。相比之下，Window 端的高分屏适配就要复杂很多，目前采用了将字体回调到小字号的办法来暂时适配高分屏，这种做法的缺陷是字体稍有有一点儿模糊。Windows 端高分屏适配除了字体问题外，还有某些图标的显示上存在问题。

### Chrome 开发者工具

增加 `F12` 快捷键打开开发者工具，用以调整笔记格式以及未来用作 `JavaScript` 插件的调试器。

### 多标签浏览文档和网页

通过 `QTabWidget` 部件实现多标签浏览文档和网页的功能。

![tabViewer](documents/images/tabViewer.png?raw=true)

### 外部编辑器

在编辑按钮右侧新增了一个菜单，用于添加和使用外部编辑器。

![extenalEditor](documents/images/external_editor.png?raw=true)

上图展示了使用 Typora 作为外部编辑器打开笔记的示意图。

## Project Plan

- [x] 文档页面增加 Devtools 。
- [x] 多标签页功能：使用 `QTabWidget` 实现多标签，将WizDocumentView部件嵌入WizMainTab的Page中，新建 `WizWebsiteView` 类，包裹 `WizWebEngineView` 用于浏览普通网页。
- [x] 添加外外部编辑器功能，用 `QFileSystemWatcher` 监控文件变动。
- [ ] 建立插件系统，包括 Global Plugin 、ExecuteScript Plugin 、HtmlDialog Plugin 、QML Plugin 、C++/Qt Plugin 、 WebSocket 等形式的插件。
- [ ] Windows 端插件添加 QAxObject 接口
- [ ] 建立其他云服务系统，如 OneDrive、Ｎextcloud 等
- [ ] 实现 NodeJs-like APIs
