# WizNotePlus v2.8.3 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## New features

* External editors can now be launched using custom shortcuts.
* Sync updates of WizQTClient internal editor:
    * Handwriting notes
    * Document markup
    * Outline notes
* The internal rich text editor can now set line height, line spacing and page paddings.

## Bug fixes

* Fixed the issue of not being able to check the to-do list in reading mode.
* Fixed the issue that the internal rich text editor title level shortcut (Ctrl+1~6) does not work.
* Fixed the problem that the URL in the note meta information was missing when copying notes.

## Known Issues

- On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
- On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76
- Document markup will show position shifts on clients of different platforms.