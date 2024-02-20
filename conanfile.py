from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.apple import XcodeDeps


class CppDigitalTwinRecipe(ConanFile):
    name = "CppDigitalTwin"
    version = "2312"
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
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("boost/1.83.0")
        self.requires("gtest/1.14.0")
        self.requires("redboltz-mqtt_cpp/13.2.1")
        self.requires("libcurl/8.4.0")
        self.requires("nlohmann_json/3.11.3")
    
    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)

        if(self.options.shared == True):
            tc["BUILD_SHARED_LIBS"]=1

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def configure(self):
        self.options["redboltz-mqtt_cpp/*"].shared = True

    

    
