# DigitalTwinKi4Boarnet

## Structure
The Project is splitted into multible Projects:
 - RealTwester:
  - This is the project containing the driver for the energyprobe and the data collection and sending.
 - caiman:
  - The original project of the arm people, that conains their driver for the energy probe. This is just a mirror of their project, that might be deleted.
 - DigiTwester:
  - The future digital twin testing programm for the architectures.

## Getting started
### Cloning
 1. Clone the repo ```git clone https://cpsgit.informatik.uni-kl.de/open/sysmd.git```
 2. Init the submodules with ```git submodule init```, this step will be eliminated
 3. Clone the submodules with the command: ```git submodule update```, this step will also be eliminated
 4. Install the dependencies with conan
 5. Build this project with your favorite CMake (>3.15) installation


## Test and Deploy


