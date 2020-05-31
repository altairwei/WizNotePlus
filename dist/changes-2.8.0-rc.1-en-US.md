# WizNotePlus v2.8.0-rc.1 Release Notes

## System Requirements

To fix #117 and #112 issues, precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

More details: [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.14/supported-platforms.html).

## New Features

* Add a new API `WizDocument::SaveToFolder`.
* Revise information on "About WizNote" dialog.

## Bug Fixes

* Wrong behaviors of 'COPY' and 'DELETE' action of internal editor.

## Known Issues

- #88: On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident.
- #76: On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start.