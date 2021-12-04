#!/usr/bin/env python

import re
import argparse

DEV_STAGE = ("alpha", "beta", "rc", "stable")

def bump_version(old_version, version_type):
    version_string, develop_string = old_version.split("-")
    version_list = version_string.split(".")
    if len(version_list) != 3:
        raise Exception("Invalid version string:", version_string)
    develop_list = develop_string.split(".")
    if len(develop_list) != 2:
        raise Exception("Invalid develop stage string:", develop_string)

    if version_type == "major":
        version_list[0] = str(int(version_list[0]) + 1)
        version_list[1] = "0"
        version_list[2] = "0"
    elif version_type == "minor":
        version_list[1] = str(int(version_list[1]) + 1)
        version_list[2] = "0"
    elif version_type == "patch":
        version_list[2] = str(int(version_list[2]) + 1)
    elif version_type == "stage":
        index = DEV_STAGE.index(develop_list[0])
        if index < len(DEV_STAGE) - 1:
            develop_list[0] = DEV_STAGE[index + 1]
            develop_list[1] = "0"
        elif index == len(DEV_STAGE) - 1:
            develop_list[0] = DEV_STAGE[0]
            develop_list[1] = "0"
    elif version_type == "num":
        develop_list[1] = str(int(develop_list[1]) + 1)

    return "-".join([".".join(version_list), ".".join(develop_list)])

def bump_file(filename, regex, version_type):
    with open(filename, encoding="UTF-8") as f:
        content = f.read()
        p = re.compile(regex)
        matched = p.search(content)
        if matched:
            version_string = matched.group(1)
            new_version = bump_version(version_string + "-stable.0", version_type)
            new_version = new_version.split("-")[0]
            replace_content = matched.group(0).replace(version_string, new_version)
            new_content = p.sub(replace_content, content, count=1)
            print("Bump %s: from" % filename, version_string, "to", new_version)
            with open(filename, "w", encoding="UTF-8") as w:
                w.write(new_content)

def bump_conanfile(version_type):
    with open("./conanfile.py", encoding="UTF-8") as f:
        conanfile_content = f.read()
        p = re.compile(r'wiznoteplus_version = "(.*)"')
        matched = p.search(conanfile_content)
        if matched:
            version_string = matched.group(1)
            new_version = bump_version(version_string, version_type)
            new_content = p.sub('wiznoteplus_version = "%s"' % new_version, conanfile_content, count=1)
            print("Bump ./conanfile.py: from", version_string, "to", new_version)
            with open("./conanfile.py", "w", encoding="UTF-8") as w:
                w.write(new_content)


def bump_wizdef(version_type):
    with open("./src/WizDef.h", encoding="UTF-8") as f:
        wizdef_content = f.read()
        p_version = re.compile(r'#define WIZ_CLIENT_VERSION  "(.*)"')
        version_matched = p_version.search(wizdef_content)
        p_devstage = re.compile(r'#define WIZ_DEV_STAGE "(.*)"')
        devstage_matched = p_devstage.search(wizdef_content)
        if version_matched and devstage_matched:
            version_string = version_matched.group(1) + "-" + devstage_matched.group(1)
            new_version = bump_version(version_string, version_type)
            new_content = p_version.sub('#define WIZ_CLIENT_VERSION  "%s"' % new_version.split("-")[0], wizdef_content, count=1)
            new_content = p_devstage.sub('#define WIZ_DEV_STAGE "%s"' % new_version.split("-")[1], new_content, count=1)
            print("Bump ./src/WizDef.h: from", version_string, "to", new_version)
            with open("./src/WizDef.h", "w", encoding="UTF-8") as w:
                w.write(new_content)

def bump_readme(version_type):
    readme_files = ["./README.md", "./doc/README-en.md"]
    for filename in readme_files:
        bump_file(
            filename,
            r"release-v(\d+\.\d+\.\d+)-green",
            version_type
        )

def bump_infoplist(version_type):
    bump_file(
        "./build/osx/Info.plist",
        r'<key>CFBundleShortVersionString</key>\n\s*<string>(\d+\.\d+\.\d+)</string>',
        version_type
    )

def bump_cmakelist(version_type):
    bump_file(
        "./CMakeLists.txt",
        r'project\(WizNotePlus VERSION (\d+\.\d+\.\d+)\)',
        version_type
    )

def bump_desktopfile(version_type):
    bump_file(
        "./build/common/wiznote2.desktop",
        r'X-AppImage-Version=(\d+\.\d+\.\d+)',
        version_type
    )

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Bump version of WizNotePlus.')
    parser.add_argument('version_type', metavar='VERTYPE', type=str,
        choices=["major", "minor", "patch", "stage", "num"], help='major, minor, patch, stage or num')
    args = parser.parse_args()
    bump_conanfile(args.version_type)
    bump_wizdef(args.version_type)
    bump_readme(args.version_type)
    bump_infoplist(args.version_type)
    bump_cmakelist(args.version_type)
    bump_desktopfile(args.version_type)