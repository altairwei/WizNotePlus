# WizNotePlus v2.8.0-beta.3

## System Requirements

Now, precompiled WizNotePlus package is built against Qt 5.12.8, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.12
* Windows: Windows 7

More details: [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.12/supported-platforms.html).

## New Features

* Add a new API `WizCommonUI::Base64ToFile`.
* Add more software build information to "About WizNote" dialog.

## Bug Fixes

* Introduce a wrong Chinese translation to the name of "Edit" menu bar by mistake.
* Unable to load note templates on Windows.
* URL of uploaded images were not changed to relative one. #106

## Known Issues

- #88: On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident.
- #76: On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start.