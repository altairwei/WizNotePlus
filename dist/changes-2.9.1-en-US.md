# WizNotePlus v2.9.1 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## Bug fixes

* Fixed the problem that the editor toolbar button icon is not updated. #192 by @notplus
* Fixed the wrong behavior of tray icon message notification on Linux system. #194
* Fixed the problem that the main window is activated when an external editor or a separate note window is saved, thus interrupting the editing experience. #141
* Fixed the problem that the automatic saving process did not trigger the content change signal when the separate note window was closed. #141
* Fixed some problems when launching and saving external editors. #199
* Fixed the problem that lite/markdown type notes cant not be rendered. #198
* Fixed the failure of the "discard changes" function of the internal editor.

## Known Issues

* On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
* On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76
* Document markup will show position shifts on clients of different platforms.
* Notes cannot be saved occasionally when the separate document window is closed.
