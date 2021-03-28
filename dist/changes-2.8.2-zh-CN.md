# WizNotePlus v2.8.2 更新日志

## 系统要求

预编译的 WizNotePlus 软件包基于 Qt 5.14.2 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.13
* Windows 最低要求 Windows 7

详情可见 [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.14/supported-platforms.html) 。

## 新的特征

* 新增 “Ctrl+W” 快捷键用于关闭当前标签页。
* 新增 “Alt+1~9” 快捷键用于切换到相应标签页。
* “隐藏侧边栏” 的快捷键从 “Alt+Ctrl+S” 变更为 “F3”。
* 现在可以通过 “Ctrl+Up/Down” 快捷键或者 “Ctrl+鼠标滑轮” 缩放页面。
* 页面标签右键菜单栏新增 “笔记定位” 和 “复制笔记内链” 选项。 #51, #89
* 当用户移动文件夹时会弹出一个确认对话框，避免不小心手滑。
* 新增 “使用桌面默认浏览器打开所有超链接” 设置选项，另外默认情况下可以通过 “Ctrl+鼠标左键” 使用桌面浏览器打开超链接。#132
* 笔记编辑器标题栏按钮的快捷键发生变更，“编辑/阅读” 快捷键变更为 “Alt+R”。其他按钮的快捷键默认被移除，但可以在 `wiznote.ini` 中新增如下内容来定制它们的快捷键。
    ```ini
    [Shortcut]
    EditNote=Alt+X
    EditNoteSeparate=Alt+P
    ```
    所有可用的按钮名称为：EditNote, EditNoteSeparate, EditNoteTags, EditShare, EditNoteInfo, EditNoteAttachments, ShowComment
* 软件启动时的欢迎页面样式进行了一些微调，并且现在页面内容能够自动更新。

## 问题修复

* 修复使用 Qt 5.9 编译本项目时的问题，某些功能会因此而禁用。

## 已知问题

- Windows 平台外部编辑器 Typora 保存时，有时会出现 WizNote 不同步更新的状况。#88
- MacOS 下“显示系统托盘图标”设置会导致程序崩溃。#76