# CM730

Firmware for the CM730

Original code from ROBOTIS. Modifications made by University of Bonn for the Nimbro-OP.

# Changes made in this repository

- Converted to a CMake build system
- CMake build system is configurable and only produces the build products necessary for flashing to the CM730/CM740 using either a 3 cell or a 4 cell battery
- Minor code changes to allow battery type to be specified as a command line preprocessor definition
- Minor code changes to support compiler versions up to `arm-none-eabi-gcc-7`
