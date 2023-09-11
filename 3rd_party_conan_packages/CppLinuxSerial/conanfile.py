from conan import ConanFile
from conan.tools import build, files
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.errors import ConanInvalidConfiguration
import os

required_conan_version = ">=1.43.0"

class CppLinuxSerialConan(ConanFile):
    name = "cpplinuxserial"
    description = ""
    license = "BSL-1.0"
    url = "https://github.com/gbmhunter/CppLinuxSerial"
    homepage = "https://github.com/gbmhunter/CppLinuxSerial"
    topics = ("mqtt", "boost", "asio")
    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False], "fPIC": [True, False], "build_tests": [True,False]}
    default_options = {"shared": True, "fPIC": True, "build_tests": True}
    no_copy_source = True

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    # def requirements(self):
    #     self.requires("boost/1.82.0")

    # def package_id(self):
        # self.info.header_only()

    # @property
    # def _compilers_minimum_version(self):
    #     return {
    #         "gcc": "6",
    #         "Visual Studio": "15.0",
    #         "clang": "5",
    #         "apple-clang": "10",
    #     }
    
    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            build.check_min_cppstd(self, 14)

            
    def source(self):
        files.get(conanfile=self,url="https://github.com/gbmhunter/CppLinuxSerial/archive/refs/tags/v2.8.0.tar.gz",
                  destination=self._source_subfolder, strip_root=True)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)

        if(self.options.build_tests):
            tc.variables["BUILD_TESTS"]="ON"
        else:
            tc.variables["BUILD_TESTS"]="OFF"


        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder=self._source_subfolder)
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        files.copy(conanfile=self,pattern="LICENSE_1_0.txt", dst="licenses", src=self._source_subfolder)
        files.copy(conanfile=self,pattern="*.hpp", dst="include", src=os.path.join(self._source_subfolder, "include"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "cpp_serial")
        self.cpp_info.set_property("cmake_target_name", "cpp_serial::cpp_serial")

        #  TODO: to remove in conan v2 once cmake_find_package_* generators removed
        self.cpp_info.filenames["cmake_find_package"] = "cpp_serial"
        self.cpp_info.filenames["cmake_find_package_multi"] = "cpp_serial"
        self.cpp_info.names["cmake_find_package"] = "cpp_serial"
        self.cpp_info.names["cmake_find_package_multi"] = "cpp_serial"
