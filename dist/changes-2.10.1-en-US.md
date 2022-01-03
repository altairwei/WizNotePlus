# WizNotePlus v2.10.1 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## Bug fixes

* Updated CryptoPP to v8.5.0 to resolve an issue that prevented WizNotePlus from running on Apple's M1 Macs via Rosetta 2.
* Fixed underline style for Welcome page notes links.
* Updated framelesshelper to fix mouse tracking BUG in title bar. wangwenx190/framelesshelper#92

## Known Issues

* On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
* On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76
* Document markup will show position shifts on clients of different platforms.
* Notes cannot be saved occasionally when the separate document window is closed.
