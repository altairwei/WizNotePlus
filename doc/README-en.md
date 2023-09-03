# WizNotePlus

forked from [WizTeam/WizQTClient](https://github.com/WizTeam/WizQTClient)

README [中文](../README.md) | **English**

**WizNotePlus** is a cross-platform cloud based note-taking client.

## Introduction

The **WizNotePlus** project is dedicated to improving the usability of cross-platform client **[WizQTClient](https://github.com/WizTeam/WizQTClient)** to meet or exceed that of the Windows platform-specific client. In order to reach the goal, we need to implement a developer-friendly plugin system and promote the establishment of a third-party developer community with good atmosphere. This project is initiated and maintained by third-party developers in compliance with the GPLv3 agreement. Any developer or user who wishes to participate in the project is welcome to contact `altair_wei@outlook.com` .

[![release](https://img.shields.io/badge/release-v2.12.0-green.svg)](https://github.com/altairwei/WizNotePlus/releases) [![license](https://img.shields.io/badge/license-GPLv3-green.svg)](https://github.com/altairwei/WizNotePlus/blob/master/LICENSE) [![build](https://github.com/altairwei/WizNotePlus/actions/workflows/build.yml/badge.svg)](https://github.com/altairwei/WizNotePlus/actions/workflows/build.yml)

## Features

**WizNotePlus** has the following features:

* Adapt to Linux/Windows HiDPI Screen
* Support for Chrome Developer Tools
* Multi-tab Documents and Websites Browser
* External Editor
* MaoXian Web Clipper supports WizNoteplus
* JavaScript plugin system
* Full screen mode

For details, please see [Main Features of the Client](https://github.com/altairwei/WizNotePlus/wiki/%E5%AE%A2%E6%88%B7%E7%AB%AF%E4%B8%BB%E8%A6%81%E7%89%B9%E6%80%A7)

## Download

Go to the [Releases](https://github.com/altairwei/WizNotePlus/releases) page of this project , and then select downloads based on your platform or needs.

## Usage

### DMG

Under the macOS platform, double-click `WizNotePlus-mac.dmg` and drag the application package to the appropriate location in the pop-up window.

### AppImage

After adding the executable permissions to `WizNotePlus-linux-x86_64.AppImage` on the Linux platform, you can run it directly. If you want to integrate AppImage into your desktop system, please refer to this [wiki page](https://github.com/altairwei/WizNotePlus/wiki/AppImage%E6%95%B4%E5%90%88%E5%85%A5%E6%A1%8C%E9%9D%A2%E7%8E%AF%E5%A2%83).

### Zip

Under Windows platform, extract the contents of the `WizNotePlus-windows.zip` to any location, then double-click `WizNote/bin/WizNote.exe` to open the client.

### Compile

If pre-compiled program do not work properly on a particular platform, you can consider compiling from source. See the project [wiki](https://github.com/altairwei/WizNotePlus/wiki/%E5%AE%A2%E6%88%B7%E7%AB%AF%E7%BC%96%E8%AF%91%E6%AD%A5%E9%AA%A4) for specific compilation steps.

## Plans

- [x] Support for Chrome Developer Tools
- [x] Multi-tab Documents and Websites Browser
- [x] External Editor
- [x] JavaScript plugin system
- [ ] Document Types and Custom Handlers
- [ ] Skin or theme system
- [ ] Local full-text search engine
- [ ] Batch Import and Export
- [ ] Fully offline portable client
- [ ] Multi-account simultaneous login
- [ ] Replace the default rich text editor
- [ ] Establish other cloud service systems

## Dependencies

- Qt 5.14 (L-GPL v3)
- QuaZIP (L-GPL V2.1)
- Cryptopp (Boost Software License 1.0)
- appdmg (MIT License)
