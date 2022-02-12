# WizNotePlus v2.10.2 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## Bug fixes

The following updates were sponsored by @JingbenShi668

* Disable command+up/down shortcuts on Mac. #214
* Lower the minimum width limit of the main window. #215
* Add "Close" button to the "Search and Replace Dialog" of the internal rich text editor. #218
* Modify rich text editor code block style to fix text overflow issue. #216

## Known Issues

* On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
* On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76
* Document markup will show position shifts on clients of different platforms.
* Notes cannot be saved occasionally when the separate document window is closed.
