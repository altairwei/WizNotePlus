# WizNotePlus v2.11.0 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## New features

* Initial implementation of notes batch export (thanks @PikachuHy), more feature requests please feedback in Github Issues. #182
* New JavaScript plugin type to enhance Wiz.Note.Outline functionality.
* Support for creating, reading and editing collaboration notes. #222
* New built-in web page zoom widget, plugin Wiz.Note.Zoom will be retired.
* Change page scrollbar style.

## Bug fixes

* Fix older version of QWebEngine crashing on GLIBC >= 2.34. #224
* Fix sorting issue with group notes folder.

## Known Issues

See [Known issues](/dist/known-issues-en-US.md) document.
