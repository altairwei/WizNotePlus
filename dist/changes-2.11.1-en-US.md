# WizNotePlus v2.11.1 Release Notes

## System Requirements

Now precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

## Bug fixes

* Fix the resolution problem of user avatar.
* Fix the icon resolution issue when dragging notes.
* Do not allow opening collaboration notes in separate window.
* Improve data synchronization experience.
    * Sync errors are no longer frequently notified in case of network disconnection or server unavailability.
    * Modify network request wait time to 60 seconds to avoid synchronization thread lock.
    * Allow forced termination of the sync threads when exiting the program.
* Improve the experience of inserting attachments in notes.
    * Fix resolution issue with thumbnail generation when dragging and dropping to insert attachments.
    * Allow generating attachment thumbnails from the attachment list and copying them to the document.
    * Allow copying files directly and then "Paste" inserting attachments from the document context menu.
* Fix the display issue of page zoom widget.

## Known Issues

See [Known issues](/dist/known-issues-en-US.md) document.
