# DigitalTwinKi4Boarnet

## Structure


## Getting started
### Cloning
 1. Clone the repo ```git clone https://cpsgit.informatik.uni-kl.de/open/sysmd.git```
 2. With the C++ Kernel Version nothing more needed.

### Conan
What is conan? Conan is a C++ package manager allowing us to change packages and also allows to install multible versions of one Package.
 1. Install conan. See therefore conan.io
 2. Create build folder within your source folder ```mkdir build && cd build```
 3. Install packages ```conan install .. -s build_type=Debug```
 4. Generate all build files from conan with your first build. ```conan build .. -s build_type=Debug```



