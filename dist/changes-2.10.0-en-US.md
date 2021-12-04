# WizNotePlus v2.10.0 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## New features

* Use [framelesshelper v2.0](https://github.com/wangwenx190/framelesshelper) as the borderless window implementation.
* Use QSS to define window styles with a new UI design, "Use system window style" setting is no longer available.
* Added tooltips for document and folder.
* Allowed users to set the interface font for the main window.
* Application renamed to WizNotePlus to avoid conflicts with WizNote X.
* New "Recent Read" section on the Welcome page.
* Added "superscript/subscript" function to internal rich text editor. #188 by @notplus
* Environment variable `WIZNOTE_DATA_STORE` is provided to replace `~/.wiznote` to set the WizNotePlus data folder location.

## Bug fixes

* Fixed the issue that the closing of the last visible window causes the application to exit. #195

## Known Issues

* On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
* On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76
* Document markup will show position shifts on clients of different platforms.
* Notes cannot be saved occasionally when the separate document window is closed.
