# WizNotePlus v2.8.2 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

More details: [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.14/supported-platforms.html).

## New features

* Add "Ctrl+W" shortcut key to close the current tab.
* Add "Alt+1~9" shortcut keys to switch to the corresponding tabs.
* The shortcut key for "Hide Sidebar" has been changed from "Alt+Ctrl+S" to "F3".
* Now you can zoom the page by "Ctrl+Up/Down" shortcut or "Ctrl+mouse wheel".
* Add "Locate this document in category" and "Copy internal link" options to the right-click menu of the page tab. #51, #89
* A confirmation dialog will pop up when user moves a folders to avoid accidents.
* Add "Open all links with desktop browser" preference option, and by default you can open hyperlinks with desktop browser by "Ctrl+left mouse button". #132
* The shortcut keys for title-bar buttons of document editor has been changed, the "Edit/Read" shortcut key has been changed to "Alt+R". The shortcuts for the other buttons have been removed by default, but they can be customized by adding the following to `wiznote.ini`.
    ```ini
    [Shortcut]
    EditNote=Alt+X
    EditNoteSeparate=Alt+P
    ```
    All available buttons are named: EditNote, EditNoteSeparate, EditNoteTags, EditShare, EditNoteInfo, EditNoteAttachments, ShowComment
* The style of the welcome page has been fine-tuned and the page content is now automatically updated.

## Bug fixes

* Fix an error when compiling this project with Qt 5.9, so some features are disabled.

## Known Issues

- On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
- On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76