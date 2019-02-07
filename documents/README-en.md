# WizNotePlus

forked from [WizTeam/WizQTClient](https://github.com/WizTeam/WizQTClient)

README [中文](../README.md) | **English**

**WizNotePlus** is a cross-platform cloud based note-taking and time management client.

## Introduction

The **WizNotePlus** project is dedicated to improving the usability of cross-platform client **[WizQTClient](https://github.com/WizTeam/WizQTClient)** to meet or exceed that of the Windows platform-specific client. In order to reach the goal, we need to implement a developer-friendly plugin system and promote the establishment of a third-party developer community with good atmosphere. This project is initiated and maintained by third-party developers in compliance with the GPLv3 agreement. Any developer or user who wishes to participate in the project is welcome to contact `altair_wei@outlook.com` .

[![release](https://img.shields.io/badge/release-v2.7.3-green.svg)](https://github.com/altairwei/WizNotePlus/releases) [![license](https://img.shields.io/badge/license-GPLv3-green.svg)](https://github.com/altairwei/WizNotePlus/blob/master/LICENSE)

## Features

### Adapt to Linux/Windows HiDPI Screen

Fixed the problem that the Linux client could not detect the correct DPI under the HiDPI screen, and also made a simple adaptation of the Window client under the HiDPI screen.

### Support for Chrome Developer Tools

Add the `F12` shortcut to open the developer tools, which can be used to adjust the note format or debug `JavaScript` plugins in the future.

### Multi-tab Documents and Websites Browser

![tabViewer](../documents/images/tabViewer.png?raw=true)

### External Editor

To add or make use of external editors, a menu has been added to the right side of the "edit button".

![extenalEditor](../documents/images/external_editor.png?raw=true)

## Download

Go to the [Releases](https://github.com/altairwei/WizNotePlus/releases) page of this project , and then select downloads based on your platform or needs.

## Usage

### DMG

Under the macOS platform, double-click `WizNote-mac.dmg` and drag the application package to the appropriate location in the pop-up window.

### AppImage

After adding the executable permissions to `WizNote-linux-x86_64.AppImage` on the Linux platform, you can run it directly. If you want to integrate AppImage into your desktop system, please refer to this [wiki page](https://github.com/altairwei/WizNotePlus/wiki/AppImage%E6%95%B4%E5%90%88%E5%85%A5%E6%A1%8C%E9%9D%A2%E7%8E%AF%E5%A2%83).

### Compile

If pre-compiled program do not work properly on a particular platform, you can consider compiling from source. See the project [wiki](https://github.com/altairwei/WizNotePlus/wiki/%E5%AE%A2%E6%88%B7%E7%AB%AF%E7%BC%96%E8%AF%91%E6%AD%A5%E9%AA%A4) for specific compilation steps.

## Plans

- [x] Support for Chrome Developer Tools
- [x] Multi-tab Documents and Websites Browser
- [x] External Editor
- [ ] JavaScript plugin system
- [ ] Replace the default rich text editor
- [ ] Establish other cloud service systems
- [ ] Skin or theme system

## Dependencies

- Qt 5.11 (L-GPL v3)
- QuaZIP (L-GPL V2.1)
- Cryptopp (Boost Software License 1.0)
- gumbo-query (MIT License)
- gumbo-parser (Apache License 2.0)
- create-dmg (MIT License)
