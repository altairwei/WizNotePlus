# WizNotePlus v2.12.0 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## New Features

* Allow users to show or hide the three sections of the main window through the "View > Layout >..." options in the main menu.
* Introduced a new ExecuteScript plugin type to execute JavaScript scripts synchronously.
* Added a series of APIs to the JavaScript plugin system:
  * Methods to run external programs such as `RunExe`, `RunProc`, and `CreateProcess`.
  * Interaction methods with users like `ShowMessage`, `Confirm`, `GetIntValue`, `GetDoubleValue`, `InputBox`, `InputMultiLineText`, `SelectItem`, and `OpenMessageConsole`.
* Allow the creation of menu-type JavaScript plugins on the main window toolbar.
* Introduced a JavaScript console for batch operations and API exploration.

## Bug fixes

* Fixed some localization errors in the interface.

## Known Issues

See [Known issues](/dist/known-issues-en-US.md) document.