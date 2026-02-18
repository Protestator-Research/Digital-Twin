from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.env import Environment
from conan.tools.apple import XcodeDeps
import os
from sys import platform


class CppDigitalTwinRecipe(ConanFile):
    name = "digitaltwin"
    version = "2501"
    package_type = "application"

    # Optional metadata
    license = "<Put the package license here>"
    author = "Moritz Herzog <herzogm@rptu.de>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of digitwester package here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": True, "fPIC": False}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "CppDigitalTwin/*"

    def requirements(self):
        self.requires("boost/[>=1.86.0 <2]")
        self.requires("libcurl/[>=8.4.0 <9]")
        self.requires("nlohmann_json/[>=3.11.3 <3.13]")
        self.requires("async-mqtt/10.2.8")
        self.requires("date/3.0.4")
        self.requires("qt/6.8.3")
        self.requires("md4c/0.5.2")
        self.requires("sysmllib/1.0-beta-4-main")
        self.requires("yaml-cpp/0.8.0")
        self.requires("openssl/3.6.0")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            # If os=Windows, fPIC will have been removed in config_options()
            # use rm_safe to avoid double delete errors
            self.options.rm_safe("fPIC")
            self.options["boost/*"].shared = True
            self.options["gtest/*"].shared = True
            self.options["redboltz-mqtt_cpp/*"].shared = True
            self.options["libcurl/*"].shared = True
            self.options["nlohmann_json/*"].shared = True
            self.options["date/*"].shared = True
            self.options["antlr4-cppruntime/*"].shared = True
            self.options["sysmllib/*"].shared=True
            self.options["qt/*"].shared = True
        else:
            self.options["boost/*"].shared = False
            self.options["gtest/*"].shared = False
            self.options["redboltz-mqtt_cpp/*"].shared = True
            self.options["libcurl/*"].shared = False
            self.options["nlohmann_json/*"].shared = False
            self.options["date/*"].shared = False
            self.options["antlr4-cppruntime/*"].shared = False
            self.options["sysmllib/*"].shared=False
            self.options["qt/*"].shared = False


        self.options["qt/*"].qtcharts = True
        self.options["qt/*"].qthttpserver = True

    
    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.user_presets_path = 'CMakePresets.json'
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.30.0 <5]")
        self.test_requires("gtest/[>=1.14.0 <2]")

    def test(self):
        cmd = os.path.join(self.cpp.build.bindir, "CppDigitalTwin/CpsBaseLib/tests/CpsBaseLibTests")
        self.run(cmd, env="conanrun")

    def package(self):
        cmake = CMake(self)
        cmake.install()

    

    
