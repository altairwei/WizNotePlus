from conans import ConanFile, CMake, tools

class WizNotePlusConan(ConanFile):
    name = "WizNotePlus"
    version = "2.7.3"
    license = "GPLv3"
    url = "https://github.com/altairwei/WizNotePlus"
    description = "A cross-platform personal knowledge and time management application."
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package", "cmake_paths", "cmake"
    requires = (
        "OpenSSL/1.0.2p@conan/stable",
        "cryptopp/5.6.5@bincrafters/stable",
        "zlib/1.2.11@conan/stable",
        "quazip/0.7.6@altairwei/testing",
        "Gumbo/0.10.1@altairwei/testing",
        "GumboQuery/latest@altairwei/testing"
    )
    options = {}
    default_options = {
        "OpenSSL:shared": True,
        "cryptopp:shared": False,
        "zlib:shared": False,
        "quazip:shared": False,
        "GumboQuery:shared": False
    }

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dll", dst="bin", src="lib")
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy("*.so*", dst="lib", src="lib")

    def build(self):
        cmake = CMake(self)
        cmake.definitions["CONAN_INSTALL_MANUALLY"] = "ON"
        cmake.configure()
        cmake.build()

    # def package(self):
    #     self.copy("*.h", dst="include", src="hello")
    #     self.copy("*hello.lib", dst="lib", keep_path=False)
    #     self.copy("*.dll", dst="bin", keep_path=False)
    #     self.copy("*.so", dst="lib", keep_path=False)
    #     self.copy("*.dylib", dst="lib", keep_path=False)
    #     self.copy("*.a", dst="lib", keep_path=False)

    # def package_info(self):
    #     self.cpp_info.libs = ["hello"]