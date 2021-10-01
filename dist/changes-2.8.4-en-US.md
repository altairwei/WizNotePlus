# WizNotePlus v2.8.4 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## New features

* Improved functionality of the search and replace dialog.
    * The default position of the search and replace window has been repositioned to the upper part of the document widget.
    * Changes in the search source line will automatically trigger a page search.
    * Pressing Ctrl+F after selecting page content will automatically search for the selected content.

## Bug fixes

* Fix an issue that stops the main screen window from being reactivated after it is closed on the MacOS platform. #175
* Fix the problem that the full-text search engine cannot search with multiple keywords. #177

## Known Issues

- On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
- On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76
- Document markup will show position shifts on clients of different platforms.