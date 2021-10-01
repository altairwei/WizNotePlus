# WizNotePlus v2.9.0 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## New features

* Pressing the mouse wheel will close the tab.
* Add "Show sub-folder documents" function for category view. #113 #157

## Known Issues

- On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
- External editor or separate editing window will cause the main interface window to be activated and displayed on the top layer when saving. #141
- On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76
- Document markup will show position shifts on clients of different platforms.