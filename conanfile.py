from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.apple import XcodeDeps


class CppDigitalTwinRecipe(ConanFile):
    name = "CppDigitalTwin"
    version = "2312"
    package_type = "application"

    # Optional metadata
    #license = "<Put the package license here>"
    author = "Moritz Herzog <herzogm@rptu.de>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of digitwester package here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("redboltz-mqtt_cpp/13.2.1")
        #if self.settings.os != "Windows":
        #    self.requires("libudev/system")
        
        #self.requires("openssl/1.1.1t")
        #self.requires("libcurl/7.79.1")
        #self.requires("nlohmann_json/3.11.2")
        #self.requires("paho-mqtt-cpp/1.2.0")
        #self.requires("cpplinuxserial/2.8.0")
    
    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
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

    

    
