[toc]

# DigitalTwinKi4Boarnet

## Structure


## Getting started
### Conan
What is conan? Conan is a C++ package manager allowing us to change packages and also allows to install multible versions of one Package.
 1. Install conan. See therefore conan.io
 2. Create build folder within your source folder ```mkdir build && cd build```
 3. Install packages ```conan install .. -s build_type=Debug```
 4. Generate all build files from conan with your first build. ```conan build .. -s build_type=Debug```



# Digital Twin Models for Testing

In this project are also multible SysMLv2 Models for the creation of the Digital Twin. The Models are stored in the following Files:

 - [Original Testing Project](Markdownfiles/DTTester4Project.md)
 - [Functional DT Project](Markdownfiles/DT4Airbag.md)
 - [AI Thread Protected DT](Markdownfiles/SecuredDT.md)
