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

### 适配高分屏

`WizCategoryViewItemBase.cpp` 中的 `drawItemBody` 会将 `iconSize` 设置成 14，当注释掉设置长宽的行为后，QIcon 的 `addFile` 并不能如文档描述般的根据给定的大小来选择不同的文件。因此，手动判断屏幕的 DPI，并判断是否在图片名称后面添加 `@2x` 后缀以选择 `24x24` 的图标.

### 待实现特征

- [ ] 文档页面增加 Devtool
- [ ] 添加外置编辑器功能，用 QFileSystemWatcher 监控文件变动
- [ ] Windows 端插件添加 QAxObject 接口
- [ ] 建立插件系统
- [ ] 建立其他云服务系统
- [ ] 跟换内核为 Electron 内核