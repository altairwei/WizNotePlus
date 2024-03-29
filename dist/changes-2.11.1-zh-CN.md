# WizNotePlus v2.11.1 更新日志

## 系统要求

预编译的 WizNotePlus 软件包基于 Qt 5.14.2 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.13
* Windows 最低要求 Windows 7

## 问题修复

* 修复用户头像的分辨率问题。
* 修复拖动笔记时图标分辨率问题。
* 不允许在 “单独窗口” 中打开协作笔记。
* 改进数据同步的体验：
    * 网络断开或服务端不可用的情况下不再频繁通报同步错误。
    * 将网络请求等待时间修改到 60 秒，避免同步线程锁定。
    * 退出程序时允许强制结束同步进程。
* 改进在笔记中插入附件的体验：
    * 修复拖拽插入附件时生成略缩图的分辨率问题。
    * 允许从附件列表生成附件略缩图并拷贝到文档中。
    * 允许直接复制文件，然后从文档右键菜单栏 “粘贴” 插入附件。
* 修复页面缩放部件的显示问题。

## 已知问题

见 [Known issues](/dist/known-issues-zh-CN.md) 文档。
