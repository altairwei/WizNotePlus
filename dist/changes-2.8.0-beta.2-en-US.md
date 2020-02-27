# WizNotePlus v2.8.0-beta.2

## New Features

- Automatically check available releases on GitHub.
- Use [conan](https://github.com/conan-io/conan) to control whole build pipeline.
- Use GitHub Actions as CI to build project.
- Add translation to some new text.
- Now WizNotePlus depends on Qt 5.14.1.

## Bug Fixes

- #34, #44 and #56: Link libraries dynamically on Linux to solve 'core dumped' issues.
- #57: can not open internal `wiz://` links.
- "Search and Replace" dialog mishandles `ENTER` key.
- External editor Chinese program file path.
- Removed extra `ClientFullscreen`  on Windows and Linux.
- #12, #38, #47, #53: Now WizNotePlus is built on Ubuntu 16.04 and rely on GLIBC_2.17.
- Disable dark mode by default on MacOS.
- Encoding error when import file into WizNote.

## Known Issues

- #88: On Windows, WizNotePlus don't sync contents with external editor such as Typora by accident.
- #76: On MacOS, disable `ShowSystemTrayIcon` will cause application crash for next start.