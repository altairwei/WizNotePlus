# WizNotePlus v2.8.1 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

More details: [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.14/supported-platforms.html).

# New features:

* A welcome page will open after the client is started.
* [Wiz.Editor.md](https://github.com/altairwei/Wiz.Editor.md/releases) plugin is ported to WizNotePlus.

## Known Issues

- On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
- On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76