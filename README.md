# WizNotePlus

forked from [WizTeam/WizQTClient](https://github.com/WizTeam/WizQTClient)

README **中文** | [English](doc/README-en.md)

**为知笔记 Plus** 是一款基于云服务的跨平台个人知识管理工具。

## 介绍

**为知笔记 Plus** 项目致力于提高 [为知笔记跨平台客户端](https://github.com/WizTeam/WizQTClient) 的可用性，以达到或者超越 Windows 平台特有客户端的水平。此项目致力于实现一个对开发者友好的插件系统，促进氛围良好的第三方开发者社区的建成。本项目在遵循 GPLv3 协议的情况下由第三方开发者发起并维护，欢迎任何有意愿参与项目贡献的开发者或用户联系 altair_wei@outlook.com 。

[![release](https://img.shields.io/badge/release-v2.12.0-green.svg)](https://github.com/altairwei/WizNotePlus/releases) [![license](https://img.shields.io/badge/license-GPLv3-green.svg)](https://github.com/altairwei/WizNotePlus/blob/master/LICENSE) [![build](https://github.com/altairwei/WizNotePlus/actions/workflows/build.yml/badge.svg)](https://github.com/altairwei/WizNotePlus/actions/workflows/build.yml)

## 为知社区

关注为知社区订阅号，获取最新信息：

![qrcode_for_gh_wizcommunity](https://github.com/altairwei/WizNotePlus/wiki/assets/qrcode_for_gh_wizcommunity.jpg)

如果你愿意促进社区发展，那[加入我们](https://github.com/altairwei/WizNotePlus/wiki/%E5%8A%A0%E5%85%A5%E6%88%91%E4%BB%AC)吧！

## 特点

**为知笔记 Plus** 主要具有以下特征：

* 适配 Linux/Windows 高分屏
* 支持 Chrome 开发者工具
* 多标签浏览文档和网页
* 外部编辑器
* MaoXian Web Clipper 浏览器网页剪裁插件
* JavaScript 插件系统
* 沉浸式全屏模式

详情请见 [客户端主要特征](https://github.com/altairwei/WizNotePlus/wiki/%E5%AE%A2%E6%88%B7%E7%AB%AF%E4%B8%BB%E8%A6%81%E7%89%B9%E6%80%A7) 。

## 下载

请前往本项目 [Releases](https://github.com/altairwei/WizNotePlus/releases) 页面，根据用户平台和需求选择下载项。

## 使用

### DMG

macOS 平台下请双击 `WizNotePlus-mac.dmg` ，并在弹出窗口中拖动应用包安装到相应位置。

### AppImage

Linux 平台下为 `WizNotePlus-linux-x86_64.AppImage` 添加可执行权限后即可运行。如果想要将 AppImage 整合入桌面系统，请参考项目维基 [AppImage 整合入桌面环境](https://github.com/altairwei/WizNotePlus/wiki/AppImage%E6%95%B4%E5%90%88%E5%85%A5%E6%A1%8C%E9%9D%A2%E7%8E%AF%E5%A2%83) 。

### Zip

Windows 平台下为 `WizNotePlus-windows.zip` ，请将压缩包内容解压到任何位置，然后双击 `WizNote/bin/WizNote.exe` 打开客户端。

### 编译

如果预编译和打包的程序无法在特定平台正常运行，可以考虑从源码编译。具体编译步骤请参见项目维基 [客户端编译步骤](https://github.com/altairwei/WizNotePlus/wiki/%E5%AE%A2%E6%88%B7%E7%AB%AF%E7%BC%96%E8%AF%91%E6%AD%A5%E9%AA%A4) 。

## 计划

- [x] Devtools 开发者工具。
- [x] 多标签浏览
- [x] 外部编辑器功能
- [x] JavaScript 插件系统
- [ ] 文档类型与自定义处理程序
- [ ] 皮肤或主题系统
- [ ] 本地全文搜索引擎
- [ ] 批量导入导出
- [ ] 完全离线便携版客户端
- [ ] 多账号同时登陆
- [ ] 更换默认富文本编辑器
- [ ] 建立其他云服务系统

## 依赖

- Qt 5.14 (L-GPL v3)
- QuaZIP (L-GPL V2.1)
- Cryptopp (Boost Software License 1.0)
- appdmg (MIT License)
