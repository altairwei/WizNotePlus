import subprocess
import os
import zipfile
import shutil
import glob
import re
import json
import contextlib
import warnings
from six import StringIO
from conans import ConanFile, CMake, tools, __version__ as conan_version
from conans.tools import Version
from conans.errors import ConanInvalidConfiguration


def qmake_query(prop, prefix=None):
    value = subprocess.check_output(
        ["qmake" if not prefix else os.path.join(prefix, "qmake"),
         "-query", prop]).decode("utf-8")
    return value.strip()

def get_qt_dir(prefix=None):
    """
    Use qmake in $PATH to locate Qt library.
    """
    return qmake_query("QT_INSTALL_PREFIX", prefix)

def get_qt_bin(prefix=None):
    """
    Use qmake in $PATH to locate deployqt tools.
    """
    return qmake_query("QT_INSTALL_BINS", prefix)

def get_qt_version(prefix=None):
    """
    Use qmake in $PATH to locate deployqt tools.
    """
    return qmake_query("QT_VERSION", prefix)

def parse_version():
    """
    Generate build version from local git repository.
    """
    wiznoteplus_version = "2.12.0-stable.0"
    if os.path.exists(".git"):
        wiznoteplus_version = subprocess.check_output(
            ["git", "describe", "--tags"]).decode("utf-8").strip()
        # Remove 'v' prefix
        wiznoteplus_version = wiznoteplus_version[1:]
        with open("version_info.txt", "w") as file:
            file.write(wiznoteplus_version)
    elif os.path.exists("version_info.txt"):
        with open("version_info.txt", "r") as file:
            wiznoteplus_version = file.read().strip()
    return wiznoteplus_version

WIZNOTEPLUS_VERSION = parse_version()

class WizNotePlusConan(ConanFile):
    name = "WizNotePlus"
    version = WIZNOTEPLUS_VERSION
    license = "GPLv3"
    url = "https://github.com/altairwei/WizNotePlus"
    description = "An enhanced community branch of WizQTClient."
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package", "cmake_paths", "cmake"
    requires = (
        "cryptopp/8.5.0",
        "zlib/1.2.11"
    )
    keep_imports = True
    options = {
        "qtdir": "ANY"
    }
    default_options = {
        "qtdir": None,
        "openssl:shared": True,
        "cryptopp:shared": True,
        "zlib:shared": True,
        "qt:shared": True,
        "qt:gui": True,
        "qt:qtsvg": True,
        "qt:qtlocation": True,
        "qt:qtwebsockets": True,
        "qt:qtwebchannel": True,
        "qt:qtdeclarative": True,
        "qt:qtwebengine": True,
        "qt:qttools": True,
        "qt:qtxmlpatterns": True,
        "qt:qttranslations": True,
        "qt:qtimageformats": True,
        "qt:qtgraphicaleffects": True,
    }
    exports_sources = (
        "CMakeLists.txt",
        "src/*",
        "share/*",
        "resources/*",
        "i18n/*",
        "cmake/*",
        "build/*",
        "tests/*",
        "LICENSE",
        "version_info.txt",
    )

    def requirements(self):
        if self.options.qtdir:
            qt_version = get_qt_version(os.path.join(str(self.options.qtdir), "bin"))
        else:
            #TODO: Current conan-qt was not ready for building QtWebEngine module
            # QtWebEngine requires python >= 2.7.5 & < 3.0.0
            self.requires("qt/5.15.6")
            qt_version = "5.15.6"
            # if tools.which("qmake"):
            #     qt_version = get_qt_version()
            # else:
            #     raise ConanInvalidConfiguration("Qt library is required!")
        # Different Qt was built against different OpenSSL
        if qt_version < Version("5.12"):
            self.requires("openssl/1.0.2u")
        else:
            self.requires("openssl/[~1.1.1m]")


    def build_requirements(self):
        if tools.os_info.is_windows and self.settings.compiler == "Visual Studio":
            self.build_requires("jom/1.1.3")

    def configure(self):
        if conan_version < Version("1.21.0"):
            raise ConanInvalidConfiguration(
                "Conan version is lower than 1.21.0")
            

    def config_options(self):

        if self.settings.os == "Windows":
            # This is a workaround of solving Error LNK2001: 
            #   WizEnc.obj : error LNK2001: unresolved external symbol 
            #   "class CryptoPP::NameValuePairs const & const CryptoPP::g_nullNameValuePairs"
            self.options["cryptopp"].shared = False
            self.options["qt"].qtwinextras = True
        elif self.settings.os == "Linux":
            self.options["qt"].qtx11extras = True
        elif self.settings.os == "Macos":
            self.options["qt"].qtmacextras = True

        if not self.options.qtdir:
            if "QTDIR" in os.environ and os.environ["QTDIR"]:
                self.options.qtdir = os.environ["QTDIR"]
            elif tools.which("qmake"):
                self.options.qtdir = get_qt_dir()
            # else:
            #     raise ConanInvalidConfiguration("Qt library is required!")


    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dll", dst="bin", src="lib")
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy("*.so*", dst="lib", src="lib", excludes = [
            "*libfcitx*",
            "*libFcitx*",
        ])

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def _configure_cmake(self):
        # Decide CMake Generateor
        if tools.os_info.is_windows and self.settings.compiler == "Visual Studio":
            gen = "NMake Makefiles JOM"
        else:
            gen = None
        # Configure CMake build system
        cmake = CMake(self, generator = gen)
        self.output.info("CMake version %s" % cmake.get_version())
        if self.options.qtdir:
            cmake.definitions["CMAKE_PREFIX_PATH"] = self.options.qtdir
        # CMakeLists.txt can be an entry point of a complete build pipline,
        #   because it will invoke conan.cmake automatically when
        #   CONAN_INSTALL_MANUALLY is OFF.
        cmake.definitions["CONAN_INSTALL_MANUALLY"] = "ON"
        cmake.configure()
        return cmake

    def package(self):
        # Internal install targets defined by CMakeLists.txt
        if tools.os_info.is_macos:
            old_appdir = os.path.join(self.build_folder, "bin", "WizNotePlus.app")
            new_appdir = os.path.join(self.package_folder, "bin", "WizNotePlus.app")
            shutil.rmtree(new_appdir, ignore_errors=True)
            shutil.copytree(old_appdir, new_appdir)
        else:
            cmake = self._configure_cmake()
            cmake.install()
        # Dynamic libraries imported by conan
        self.copy("*WizNotePlus*", src="bin", dst="bin", keep_path=True)
        self.copy("*.dll", src="bin", dst="bin", keep_path=True)
        self.copy("*.dylib", src="bin", dst="bin", keep_path=True)
        self.copy("*.so*", src="lib", dst="lib", keep_path=True)
        self.copy("*", src="plugins", dst="plugins", keep_path=True)
        # Other files
        self.copy("*.bdic", src=os.path.join("share", "wiznote", "qtwebengine_dictionaries"),
                  dst=os.path.join("bin", "qtwebengine_dictionaries"), keep_path=False)
        if tools.os_info.is_linux:
            try:
                mybuf = StringIO()
                self.run("fcitx-diagnose | grep -o '/.*/libfcitxplatforminputcontextplugin.so'", output=mybuf)
                fcitx_qt5_lib = mybuf.getvalue().strip()
                if fcitx_qt5_lib.endswith("libfcitxplatforminputcontextplugin.so"):
                    dest_folder = os.path.join(self.package_folder, "plugins", "platforminputcontexts")
                    if not os.path.exists(dest_folder):
                        os.makedirs(dest_folder)
                    shutil.copy(fcitx_qt5_lib, dest_folder)
                    self.output.success("Copied libfcitxplatforminputcontextplugin.so")
            except:
                self.output.warn("Failed to copy libfcitxplatforminputcontextplugin.so")

        # Deploy
        self._deploy()

    def _deploy(self):
        """
        This method is mainly used to deploy and package WizNotePlus.
        """
        # Create distribution
        dist_folder = os.path.join(self.package_folder, "dist")
        os.makedirs(dist_folder, exist_ok=True)
        appdir = self._create_app_dir(dist_folder)
        # Packaging Qt or other runtime libraries
        #   such as windeployqt、macdeployqt、and linuxdeployqt
        deployqt, executable, options = self._configure_deployqt(dist_folder, appdir)
        self.run("%s %s %s" % (deployqt, executable, options), cwd=dist_folder)
        # Create package
        if self.settings.os == "Windows":
            self._create_dist_archive(dist_folder)
        elif self.settings.os == "Macos":
            self._create_dist_dmg(dist_folder)
        elif self.settings.os == "Linux":
            # TODO: create AppImage or other type, such as Snap and Flatpak
            shutil.rmtree(appdir, ignore_errors=True)
            appimages = glob.glob(os.path.join(dist_folder, 'WizNotePlus*.AppImage'))
            shutil.move(appimages.pop(),
                os.path.join(dist_folder, "WizNotePlus-linux-v%s.AppImage" % self.version))
        else:
            raise Exception("Unsupported platforms: %s" % self.settings.os)

    def _create_dist_archive(self, dist_folder):
        dist_file_name = os.path.join(
            dist_folder, "WizNotePlus-windows-v%s.zip" % self.version)
        with zipfile.ZipFile(dist_file_name, 'w', zipfile.ZIP_DEFLATED) as zipf:
            # Archive "bin" and "share" into "WizNotePlus" prefix
            for folder in ("bin", "share"):
                path = os.path.join(self.package_folder, folder)
                for root, dirs, files in os.walk(path):
                    for file in files:
                        source_filename = os.path.join(root, file)
                        archive_filename = os.path.join(
                            root.replace(self.package_folder, "WizNotePlus"), file)
                        zipf.write(source_filename, archive_filename)

    def _create_dist_dmg(self, dist_folder):
        app_dir = os.path.join(self.package_folder, "bin", "WizNotePlus.app")
        dmg_json = os.path.join(self.package_folder, "bin", "dmg.json")
        self._fix_macdeployqt()
        #Change build version
        self.run(" plutil -replace CFBundleVersion "
                " -string {build_version} {info_plist} ".format(
                    build_version = self.version.split("-").pop(),
                    info_plist = os.path.join(app_dir, "Contents", "Info.plist")
                ))
        #Change RPATH
        self.run(
            "install_name_tool -add_rpath "
            "'@executable_path/../Frameworks' {ex}".format(
            ex = os.path.join(self.package_folder, 
                "bin", "WizNotePlus.app", "Contents", "MacOS", "WizNotePlus")
        ))
        # Create dmg
        with open(dmg_json, 'w') as f:
            json.dump({
                "title": "Install WizNotePlus",
                "icon": os.path.join(self.source_folder, "build", "common", "logo", "wiznote.icns"),
                "icon-size": 100,
                "background": os.path.join(self.source_folder, "resources", "wiznote-disk-cover.jpg"),
                "window": {
                    "position": {
                        "x": 200,
                        "y": 120
                    },
                    "size": {
                        "width": 522,
                        "height": 350
                    }
                },
                "contents": [
                    { "x": 400, "y": 190, "type": "link", "path": "/Applications" },
                    { "x": 120, "y": 190, "type": "file", "path": app_dir }
                ],
                "format": "UDZO"
            }, f)
        output_dmg = "%s/WizNotePlus-mac-v%s.dmg" %  (dist_folder, self.version)
        with contextlib.suppress(FileNotFoundError):
            os.remove(output_dmg)
        self.run(
            " appdmg {input} {output} ".format(
                input = dmg_json,
                output = output_dmg
            )
        )

    def _create_app_dir(self, dist_folder):
        if self.settings.os == "Linux":
            # Create AppDir in dist_folder temporarily
            appdir = os.path.join(dist_folder, "WizNotePlus.AppDir")
            usrdir = os.path.join(appdir, "usr")
            os.makedirs(appdir, exist_ok=True)
            os.makedirs(usrdir, exist_ok=True)
            # hard link all files in to AppDir
            for folder in ("bin", "lib", "plugins", "share"):
                path = os.path.join(self.package_folder, folder)
                dest_root = os.path.join(self.package_folder, usrdir)
                for root, dirs, files in os.walk(path):
                    for file in files:
                        source_filename = os.path.join(root, file)
                        dest_prefix = root.replace(self.package_folder, dest_root)
                        dest_filename = os.path.join(dest_prefix, file)
                        os.makedirs(dest_prefix, exist_ok = True)
                        os.link(source_filename, dest_filename)
            return appdir
        else:
            return None

    def _create_dist_appimage(self, dist_folder):
        # Create AppDir in dist_folder temporarily
        appdir = os.path.join(dist_folder, "WizNotePlus.AppDir")
        usrdir = os.path.join(appdir, "usr")
        os.makedirs(appdir, exist_ok=True)
        os.makedirs(usrdir, exist_ok=True)
        for file in ("wiznote.desktop", "wiznote.png", ".DirIcon"):
            shutil.move(os.path.join(self.install_folder, "..", file), os.path.join(appdir, file))
        os.remove(os.path.join(self.install_folder, "..", "AppRun"))
        # hard link all files in to AppDir
        for folder in ("bin", "lib", "libexec", "plugins", "resources", "share", "translations"):
            path = os.path.join(self.install_folder, folder)
            dest_root = os.path.join(self.install_folder, usrdir)
            for root, dirs, files in os.walk(path):
                for file in files:
                    source_filename = os.path.join(root, file)
                    dest_prefix = root.replace(self.install_folder, dest_root)
                    dest_filename = os.path.join(dest_prefix, file)
                    os.makedirs(dest_prefix, exist_ok = True)
                    os.link(source_filename, dest_filename)
        os.symlink(os.path.join("usr", "bin", "WizNotePlus"), os.path.join(appdir, "AppRun"))
        # Use appimagetool to create AppImage
        self.run(
            "appimagetool --appimage-extract-and-run {source} " 
            " WizNotePlus-linux-v{version}.AppImage".format(
                source = appdir, version = self.version
            ), cwd=dist_folder)
        # Remove temporary links
        shutil.rmtree(appdir, ignore_errors=True)

    def _fix_dependencies_name(self, target, libs):
        deps = subprocess.check_output(["otool", "-L", target]).decode("utf-8")
        for libname in libs:
            lib_basename = os.path.basename(libname)
            matched = re.search(r'\n\t(.*%s)' % lib_basename, deps)
            if matched:
                old_id = matched.group(1)
                new_id = "@executable_path/../Frameworks/%s" % os.path.basename(libname)
                self.run("install_name_tool -change {old_id} {new_id} {target}".format(
                    old_id = old_id, new_id = new_id, target = target))
            else:
                warnings.warn("%s not found in dependencies of %s" % (lib_basename, target))

    def _fix_install_name(self, libs):
        #TODO: Trim dirname of dylib, and prepend @executable_path/../Frameworks
        for libname in libs:
            new_id = "@executable_path/../Frameworks/%s" % os.path.basename(libname)
            self.run("install_name_tool -id %s %s" % (new_id, libname))

    def _fix_macdeployqt(self):
        app_dir = os.path.join(self.package_folder, "bin", "WizNotePlus.app")
        fram_dir = os.path.join(app_dir, "Contents", "Frameworks")
        # Copy libs to bundle
        shutil.copy(
            os.path.join(self.package_folder, "bin", "libcryptopp.8.3.dylib"),
            os.path.join(fram_dir, "libcryptopp.8.3.dylib")
        )
        shutil.copy(
            os.path.join(self.package_folder, "bin", "libz.1.dylib"),
            os.path.join(fram_dir, "libz.1.dylib")
        )
        # Change install name
        self._fix_install_name([
            os.path.join(fram_dir, "libcryptopp.8.3.dylib"),
            os.path.join(fram_dir, "libz.1.dylib")
        ])
        # Change dependencis list
        self._fix_dependencies_name(
            os.path.join(app_dir, "Contents", "MacOS", "WizNotePlus"), 
            ["libcryptopp.8.3.dylib"])
        #self._fix_dependencies_name(
        #    os.path.join(fram_dir, "libquazip5.1.dylib"), 
        #    ["libz.1.dylib"])

    def _configure_deployqt(self, dist_folder, appdir):
        # TODO: use conan-qt instead of checking system installed qmake.
        if self.options.qtdir:
            qt_bin = os.path.join(str(self.options.qtdir), "bin")
        else:
            qt_bin = get_qt_bin()
        if self.settings.os == "Windows":
            deployqt = os.path.join(qt_bin, "windeployqt")
            executable = os.path.join("bin", "WizNotePlus.exe")
            options = ""
        elif self.settings.os == "Macos":
            deployqt = os.path.join(qt_bin, "macdeployqt")
            executable = os.path.join(self.package_folder, "bin", "WizNotePlus.app")
            options = "-verbose=1 -executable={ex} -libpath={lib}".format(
                ex = os.path.join(executable, "Contents", "MacOS", "WizNotePlus"),
                lib = str(self.options.qtdir))
        elif self.settings.os == "Linux":
            deployqt = "linuxdeployqt"
            shutil.copy(
                os.path.join(self.source_folder, "build", "common", "wiznote2.desktop"),
                os.path.join(appdir, "wiznote.desktop"))
            executable = os.path.join(appdir, "usr", "share", "applications", "wiznote.desktop")
            options = " ".join([
                "--appimage-extract-and-run", "-verbose=1", "-appimage",
                #"-unsupported-allow-new-glibc",
                # Workaround for https://github.com/probonopd/linuxdeployqt/issues/35
                "-exclude-libs=libnss3.so,libnssutil3.so",
                "-qmake=%s" % os.path.join(qt_bin, "qmake")])
        else:
            raise Exception("Unsupported platforms: %s" % self.settings.os)

        return (deployqt, os.path.join(self.package_folder, executable), options)
