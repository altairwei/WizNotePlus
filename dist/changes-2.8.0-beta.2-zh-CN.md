# WizNotePlus v2.8.0-beta.2

## 新的特征

- 自动检查 GitHub 上的可用 WizNotePlus 更新。
- 使用 [conan](https://github.com/conan-io/conan) 控制整个项目构建流程。
- 使用 GitHub Actions 作为 CI 来自动化并发布 WizNotePlus.
- 为一些新的文本添加了中文翻译。
- WizNotePlus 现在依赖于 Qt 5.14.1 。

## 问题修复

- #34, #44, #56: 在 Linux 上动态链接库文件，以解决 Core dumped 相关问题。
- #57: 修复了无法打开 `wiz://` 内部链接的问题。
- 文档浏览页面 “搜索替换” 对话框错误处理 `ENTER` 键，导致死循环。
- 外部编辑器配置窗口现在可以输入中文路径了。
- 移除 Windows 和 Linux 上多余的 `ClientFullscreen` 菜单项。
- #12, #38, #47, #53: 现在 WizNotePlus 在 Ubuntu 16.04 上构建，最高依赖于 GLIBC_2.17 。
- 在 MacOS 平台默认禁用黑暗模式。
- 修复导入文件到 WizNote 时出现的字符编码错误。

## 已知问题

- #88: Windows 平台外部编辑器 Typora 保存时，有时会出现 WizNote 不同步更新的状况。
- #76: MacOS 下“显示系统托盘图标”设置会导致程序崩溃。