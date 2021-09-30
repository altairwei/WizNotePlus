import os
import subprocess

needed_files = []
excludes = [os.path.join("src", "libs", "3rdparty")]
for root, subdirs, subfiles in os.walk("src", topdown=True):
    subdirs[:] = [d for d in subdirs if os.path.join(root, d) not in excludes]
    for file in subfiles:
        source_filename = os.path.join(root, file)
        ext = os.path.splitext(source_filename)[1]
        if ext in [".h", ".cpp", ".ui"]:
            print("Found file: " + source_filename)
            needed_files.append(source_filename)

subprocess.run(
    [
        "lupdate",
        *needed_files,
        "-ts",
        "./i18n/wiznote_zh_CN.ts",
        "./i18n/wiznote_zh_TW.ts",
        "-no-obsolete",
        "-recursive"
    ]
)