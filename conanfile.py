import subprocess
import os
import zipfile
import shutil
from conans import ConanFile, CMake

def get_qt_dir():
    """
    Use qmake in $PATH to locate Qt library.
    """
    qt_dir = subprocess.check_output(
        ["qmake", "-query", "QT_INSTALL_PREFIX"], encoding="utf-8")
    return qt_dir.strip()

def get_qt_bin():
    """
    Use qmake in $PATH to locate deployqt tools.
    """
    qt_bin = subprocess.check_output(
        ["qmake", "-query", "QT_INSTALL_BINS"], encoding="utf-8")
    return qt_bin.strip()

def parse_version():
    """
    Generate build version from local git repository.
    """
    wiznoteplus_version = "2.8.0-beta.1"
    if os.path.exists(".git"):
        wiznoteplus_version = subprocess.check_output(
            ["git", "describe", "--tags"], encoding="utf-8").strip()
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
        "OpenSSL/1.0.2p@conan/stable",
        "cryptopp/5.6.5@bincrafters/stable",
        "zlib/1.2.11@conan/stable",
        "quazip/0.7.6@altairwei/testing"
    )
    build_requires = (
        "cmake_installer/3.12.4@conan/stable"
    )
    keep_imports = True
    options = {}
    default_options = {
        "OpenSSL:shared": True,
        "cryptopp:shared": True,
        "zlib:shared": True,
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
        if self.settings.os == "Linux":
            self.requires("linuxdeployqt/v6@altairwei/testing")
            self.requires("appimagetool/v12@altairwei/testing")
            self.requires("fcitx-qt5/1.1.1@altairwei/testing")
            self.requires("fcitx5-qt/0.0.0@altairwei/testing")
        if self.settings.os == "Macos":
            self.requires("create-dmg/1.0.0.5@altairwei/testing")

    def config_options(self):
        # This is a workaround of solving Error LNK2001: 
        #   WizEnc.obj : error LNK2001: unresolved external symbol 
        #   "class CryptoPP::NameValuePairs const & const CryptoPP::g_nullNameValuePairs"
        if self.settings.os == "Windows":
            self.options["cryptopp"].shared = False

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dll", dst="bin", src="lib")
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy("*.so*", dst="lib", src="lib", excludes = [
            "*libfcitx*",
            "*libFcitx*",
        ])
        self.copy("libfcitxplatforminputcontextplugin.so", 
            src="lib/x86_64-linux-gnu/qt5/plugins/platforminputcontexts",
            dst="plugins/platforminputcontexts",
            root_package="fcitx-qt5", keep_path=False)
        self.copy("libfcitx5platforminputcontextplugin.so", 
            src="lib/qt/plugins/platforminputcontexts",
            dst="plugins/platforminputcontexts",
            root_package="fcitx5-qt", keep_path=False)

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        # Packaging internal install tagers defined by CMakeLists.txt
        cmake = self._configure_cmake()
        cmake.install()
        # Packaging dynamic libraries imported by conan
        self.copy("*WizNote*", src="bin", dst="bin", keep_path=True)
        self.copy("*.dll", src="bin", dst="bin", keep_path=True)
        self.copy("*.dylib", src="bin", dst="bin", keep_path=True)
        self.copy("*.so*", src="lib", dst="lib", keep_path=True)
        self.copy("*", src="plugins", dst="plugins", keep_path=True)

    def deploy(self):
        """
        This method is mainly used to deploy and package WizNotePlus.
        """
        self.copy("*", dst="bin", src="bin")
        self.copy("*", dst="lib", src="lib")
        self.copy("*", dst="share", src="share")
        # Packaging Qt or other runtime libraries
        #   such as windeployqt、macdeployqt、and linuxdeployqt
        deployqt, executable, options = self._configure_deployqt()
        self.run("%s %s %s" % (deployqt, executable, options))
        #TODO: This is a workaround.
        self.copy("*", dst="plugins", src="plugins")
        # Create distribution
        dist_folder = os.path.join(self.install_folder, "dist")
        os.makedirs(dist_folder, exist_ok=True)
        if self.settings.os == "Windows":
            self._create_dist_archive(dist_folder)
        elif self.settings.os == "Macos":
            self._create_dist_dmg(dist_folder)
        elif self.settings.os == "Linux":
            # TODO: create AppImage or other type, such as Snap and Flatpak
            self._create_dist_appimage(dist_folder)
        else:
            raise Exception("Unsupported platforms: %s" % self.settings.os)

    def _create_dist_archive(self, dist_folder):
        dist_file_name = os.path.join(
            dist_folder, "WizNotePlus-windows-v%s.zip" % self.version)
        with zipfile.ZipFile(dist_file_name, 'w', zipfile.ZIP_DEFLATED) as zipf:
            # Archive "bin" and "share" into "WizNote" prefix
            for folder in ("bin", "share"):
                path = os.path.join(self.install_folder, folder)
                for root, dirs, files in os.walk(path):
                    for file in files:
                        source_filename = os.path.join(root, file)
                        archive_filename = os.path.join(
                            root.replace(self.install_folder, "WizNote"), file)
                        zipf.write(source_filename, archive_filename)

    def _create_dist_dmg(self, dist_folder):
        output_dmg = "%s/WizNotePlus-mac-v%s.dmg" %  dist_folder, self.version
        self.run(
            " create-dmg "
            " --volname wiznote-disk "
            " --background {source}/resources/wiznote-disk-cover.jpg "
            " --window-pos 200 120 "
            " --window-size 522 350 "
            " --icon-size 100 "
            " --icon 'WizNote.app' 100 190 "
            " --hide-extension 'WizNote.app' "
            " --app-drop-link 400 190 "
            " --format UDZO "
            " {output} "
            " {input} ".format(
                source = self.source_folder,
                output = output_dmg,
                input = self.install_folder
            )
        )

    def _create_dist_appimage(self, dist_folder):
        # Create AppDir in dist_folder temporarily
        appdir = os.path.join(dist_folder, "WizNote.AppDir")
        usrdir = os.path.join(appdir, "usr")
        os.mkdir(appdir)
        os.mkdir(usrdir)
        for file in ("wiznote.desktop", "wiznote.png"):
            shutil.move(os.path.join(self.install_folder, "..", file), os.path.join(appdir, file))
        os.remove(os.path.join(self.install_folder, "..", "AppRun"))
        # hadr lonk all files in to AppDir
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
        os.symlink(os.path.join("usr", "bin", "WizNote"), os.path.join(appdir, "AppRun"))
        # Use appimagetool to create AppImage
        self.run(
            "appimagetool {source} WizNotePlus-linux-v{version}.AppImage".format(
                source = appdir, version = self.version
            ), cwd=dist_folder)
        # Remove temporary links
        shutil.rmtree(appdir, ignore_errors=True)

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_PREFIX_PATH"] = get_qt_dir()
        # CMakeLists.txt can be an entry point of a complete build pipline,
        #   because it will invoke conan.cmake automatically when
        #   CONAN_INSTALL_MANUALLY is OFF.
        cmake.definitions["CONAN_INSTALL_MANUALLY"] = "ON"
        cmake.configure()
        return cmake

    def _configure_deployqt(self):
        # TODO: use conan-qt instead of checking system installed qmake.
        qt_bin = get_qt_bin()
        if self.settings.os == "Windows":
            deployqt = os.path.join(qt_bin, "windeployqt")
            executable = os.path.join("bin", "WizNote.exe")
            options = ""
        elif self.settings.os == "Macos":
            deployqt = os.path.join(qt_bin, "macdeployqt")
            executable = os.path.join("Contents", "MacOS", "WizNote")
            options = ""
        elif self.settings.os == "Linux":
            deployqt = "linuxdeployqt"
            #executable = os.path.join("bin", "WizNote")
            executable = os.path.join("share", "applications", "wiznote.desktop")
            options = "-unsupported-allow-new-glibc"
        else:
            raise Exception("Unsupported platforms: %s" % self.settings.os)

        return (deployqt, os.path.join(self.install_folder, executable), options)
