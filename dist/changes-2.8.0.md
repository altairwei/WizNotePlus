WizNotePlus 2.8.0 release contains bug fixes and new features.

New features:

* New [MaoXian Web Clipper](https://github.com/mika-cn/maoxian-web-clipper) clipping-handler for WizNotePlus, please visit [wiki](https://github.com/altairwei/WizNotePlus/wiki/MaoXian%E7%BD%91%E9%A1%B5%E5%89%AA%E8%A3%81%E6%8F%92%E4%BB%B6%E8%AE%BE%E7%BD%AE) page to see usage tips.
* Experimental JavaScript plugin system, see all [available plugins](https://github.com/altairwei/WizNotePlus/wiki/%E6%8F%92%E4%BB%B6%E5%AE%89%E8%A3%85%E6%8C%87%E5%8D%97).
* Now, we provided precompiled WizNotePlus client on Windows.
* Add immersive full screen mode; you can enter the mode by pressing F11 on Linux or Windows.
* Updated builtin rich text editor; the source code is from WizQTClient.
* Removed CocoaToolbar of WizNotePlus on MacOS; To reduce maintenance costs, the UI will be unified to Linux and Windows.
* Add "Discard changes" action to context menu of edit button when your document is in editing mode.
* Add "Close all tabs" action to context menu of document tabs.
* Add "Rename" action to context menu of document list view items.
* You can input HTTP/HTTPS address in search bar to open web page in WizNotePlus.
* Highlight location of key words on scroll bar of non-markdown document when you are searching.

Bug fixes:

* Changed icon system to svg-based one.
* Changed the way of publishing C++ object to JavaScript enviroment.
* Fixed "Import Files" can not save resources when importing files.
* Fixed font fallback error when reading documents.
* Removed the behaviour of opening `www.wiz.cn` automatically.
* Fixed the problem of arrange categories' order.