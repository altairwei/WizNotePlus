# WizNotePlus v2.8.0 更新日志

## 系统要求

为了修复 #117 和 #112 提到的问题，预编译的 WizNotePlus 软件包现在基于 Qt 5.14.2 构建，最低系统需求如下：

* Linux 最低要求 GLIBC_2.17
* MacOS 最低要求 macOS 10.13
* Windows 最低要求 Windows 7

详情可见 [Supported Platforms](https://doc-snapshots.qt.io/qt5-5.14/supported-platforms.html) 。

### 新的特征：

* Chrome/Firefox 浏览器网页剪裁插件 [MaoXian Web Clipper](https://github.com/mika-cn/maoxian-web-clipper) 新增对 WizNotePlus 的支持；安装和使用方法请参见 [MaoXian网页剪裁插件设置](https://github.com/altairwei/WizNotePlus/wiki/MaoXian%E7%BD%91%E9%A1%B5%E5%89%AA%E8%A3%81%E6%8F%92%E4%BB%B6%E8%AE%BE%E7%BD%AE) 。
* 实验性的 JavaScript 插件系统；安装方法和可用插件列表请参见 [插件安装指南](https://github.com/altairwei/WizNotePlus/wiki/%E6%8F%92%E4%BB%B6%E5%AE%89%E8%A3%85%E6%8C%87%E5%8D%97) 。
* 新增 Windows 平台下的 WizNotePlus 预编译包；解压后可直接运行。
* 沉浸式全屏模式；Linux/Windows 下按 F11 键开启，MacOS 下暂时无法使用（尚无法决定是否移除原有的Meta+Ctrl+F 键开启的全屏模式）。
* MacOS 平台下的 CocoaToolbar 被移除；UI 将与 Linux/Windows 统一，以降低维护成本。
* 编辑按钮右侧菜单在文档处于编辑状态下时会现实 “Discard changes” 选项。
* 在多标签浏览器的页面标签右键菜单中添加 “Close all tabs” 选项。
* 文档列表右键菜单中添加 “重命名” 选项。
* 搜索栏目可直接输入 HTTP/HTTPS 地址，在 WizNotePlus 中打开网页。
* 页面链接可以直接在 WizNotePlus 中打开了。
* 关键词搜索时，非 Markdown 文档将会在滚动条上显示关键词位置。
* 自动检查 GitHub 上的可用 WizNotePlus 更新。
* 使用 [conan](https://github.com/conan-io/conan) 控制整个项目构建流程。
* 使用 GitHub Actions 作为 CI 来自动化并发布 WizNotePlus.
* 为一些新的文本添加了中文翻译。
* 新增 `WizCommonUI::Base64ToFile` API。
* 新增 `WizDocument::SaveToFolder` API。
* 在 “关于 WizNote” 对话框中新增了更多的软件构建信息。

### 问题修复：

* UI 改用基于 SVG 的图标系统。
* 更改了 C++ 对象发布到 JavaScript 环境中的方式。
* 修复 “导入文件” 时相关资源文件没有被保存的问题。
* 修复文档阅读时字体回退错误。
* 移除了启动时自动打开 `www.wiz.cn` 页面。
* 在 Linux 上动态链接库文件，以解决 Core dumped 相关问题。#34, #44, #56
* 修复了无法打开 `wiz://` 内部链接的问题。#57
* 文档浏览页面 “搜索替换” 对话框错误处理 `ENTER` 键，导致死循环。
* 外部编辑器配置窗口现在可以输入中文路径了。
* 移除 Windows 和 Linux 上多余的 `ClientFullscreen` 菜单项。
* 现在 WizNotePlus 在 Ubuntu 16.04 上构建，最高依赖于 GLIBC_2.17 。#12, #38, #47, #53
* 在 MacOS 平台默认禁用黑暗模式。
* 修复导入文件到 WizNote 时出现的字符编码错误。
* 菜单栏 “编辑” 项名称在中文翻译时错误地引入了全角右括号。
* Windows 平台客户端无法加载和使用笔记模板。
* 在笔记中插入或者粘贴图片时，因为图片文件的绝对地址没有自动被修改成相对地址，导致跨平台图片显示错误。#106
* 内部编辑器复制及删除操作导致的错误行为。#117, #112

## 已知问题

- Windows 平台外部编辑器 Typora 保存时，有时会出现 WizNote 不同步更新的状况。#88
- MacOS 下“显示系统托盘图标”设置会导致程序崩溃。#76