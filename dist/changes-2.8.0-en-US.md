# WizNotePlus v2.8.0 Release Notes

## System Requirements

To fix #117 and #112 issues, precompiled WizNotePlus package is built against Qt 5.14.2, and minimum system requirements are listed below:

* Linux: GLIBC_2.17
* MacOS: macOS 10.13
* Windows: Windows 7

More details: [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.14/supported-platforms.html).

# New features:

* New [MaoXian Web Clipper](https://github.com/mika-cn/maoxian-web-clipper) clipping-handler for WizNotePlus, please visit [wiki](https://github.com/altairwei/WizNotePlus/wiki/MaoXian%E7%BD%91%E9%A1%B5%E5%89%AA%E8%A3%81%E6%8F%92%E4%BB%B6%E8%AE%BE%E7%BD%AE) page to see usage tips.
* Experimental JavaScript plugin system, see all [available plugins](https://github.com/altairwei/WizNotePlus/wiki/%E6%8F%92%E4%BB%B6%E5%AE%89%E8%A3%85%E6%8C%87%E5%8D%97).
* Now, we provided precompiled WizNotePlus client on Windows.
* Add immersive full screen mode; you can enter the mode by pressing F11 on Linux or Windows.
* Removed CocoaToolbar of WizNotePlus on MacOS; To reduce maintenance costs, the UI will be unified to Linux and Windows.
* Add "Discard changes" action to context menu of edit button when your document is in editing mode.
* Add "Close all tabs" action to context menu of document tabs.
* Add "Rename" action to context menu of document list view items.
* You can input HTTP/HTTPS address in search bar to open web page in WizNotePlus.
* Highlight location of key words on scroll bar of non-markdown document when you are searching.
* Automatically check available releases on GitHub.
* Use [conan](https://github.com/conan-io/conan) to control whole build pipeline.
* Use GitHub Actions as CI to build project.
* Add translation to some new text.
* Add a new API `WizCommonUI::Base64ToFile`.
* Add a new API `WizDocument::SaveToFolder`.
* Add more software build information to "About WizNote" dialog.

# Bug fixes:

* Changed icon system to svg-based one.
* Changed the way of publishing C++ object to JavaScript enviroment.
* Fixed "Import Files" can not save resources when importing files.
* Fixed font fallback error when reading documents.
* Removed the behaviour of opening `www.wiz.cn` automatically.
* Fixed the problem of arrange categories' order.
* Link libraries dynamically on Linux to solve 'core dumped' issues. #34, #44 and #56
* Can not open internal `wiz://` links. #57
* "Search and Replace" dialog mishandles `ENTER` key.
* External editor Chinese program file path.
* Removed extra `ClientFullscreen`  on Windows and Linux.
* Now WizNotePlus is built on Ubuntu 16.04 and rely on GLIBC_2.17. #12, #38, #47, #53
* Disable dark mode by default on MacOS.
* Encoding error when import file into WizNote.
* Introduce a wrong Chinese translation to the name of "Edit" menu bar by mistake.
* Unable to load note templates on Windows.
* URL of uploaded images were not changed to relative one. #106
* Wrong behaviors of 'COPY' and 'DELETE' action of internal editor. #117, #112

## Known Issues

- On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident. #88
- On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start. #76