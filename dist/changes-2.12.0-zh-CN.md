# WizNotePlus v2.12.0 更新日志

## 系统要求

预编译的 WizNotePlus 软件包基于 Qt 5.14.2 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.13
* Windows 最低要求 Windows 7

## 新的特征

* 允许用户通过主菜单栏的 “查看 > 布局 >...” 选项，分别显示或隐藏主界面的三个栏目。
* 新增 ExecuteScript 插件类型，以同步地执行 JavaScript 脚本。
* 为 JavaScript 插件系统新增一系列 APIs:
  * 运行外部程序的 `RunExe`、`RunProc` 以及 `CreateProcess` 等方法。
  * 与用户交互的 `ShowMessage`、`Confirm`、`GetIntValue`、`GetDoubleValue`、`InputBox`、`InputMultiLineText`、`SelectItem`、`OpenMessageConsole` 等方法。
* 允许在主界面工具栏上创建菜单类型的 JavaScript 插件。
* 新增 JavaScript 控制台以方便批量操作和探索 APIs。

## 问题修复

* 修复了界面本地化的一些错误。

## 已知问题

见 [Known issues](/dist/known-issues-zh-CN.md) 文档。
