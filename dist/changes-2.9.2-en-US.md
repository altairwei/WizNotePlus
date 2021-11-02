# WizNotePlus v2.9.2 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## Bug fixes

* Fixed position jump issue between InspectElement and OpenTempFileLocation in document view context menu.
* Sync server APIs changes made by WizQTClient.
* No longer automatically search for WizBox when login.
* Hidding "purchase vip" menu item for private deployment accounts.

## Known Issues

* On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
* On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76
* Document markup will show position shifts on clients of different platforms.
* Notes cannot be saved occasionally when the separate document window is closed.
