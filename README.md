# Open Story Teller (OST)

Open Story Teller is an Open Source project to provide guidelines and software to build your own story teller box. 

The main goal is to *not* make electronics boards but instead buy or reuse existing ones. This will allow you to easily repair your device with spare parts and avoid 

We propose a set of parts and firmware that is working well togather but your are free to custom everything to fit your needs.

*DO NOT BUILD YOUR OWN BOARD, JUST USE EXISTING ONES!*

This project can be used as a base platform for any device that is composed by:
- A display (TFT...)
- An Audio output
- A SD card or memory
- Some Buttons / rotary encoder / potentiometer

# Firmware/software

The firmware is highly configurable and highly portable. To achieve that, it is split in multiple parts:
- The core source file, which is common to every target
- the ports, dedicated to an embedded MCU and board
- The tests, to easily test part of source on a standard PC
- A desktop/mobile implementation

The core is written in pure C, targets implementations may add other languages and libraries (QML/C++/python ...).

## GCC build system

The project uses CMake as build system. For the embedded targets, the main CMakeLists.txt includes a generic cross compiler file that should be good for many configurations as soon as your compiler is GCC.

## Invocation

1. Create a build directory (mkdir build)
2. Invoke cmake with some options, passed as definitions for CMake (-D<option>)

| Option | Role |
|---|---|
| TOOLCHAIN      |   specify the prefix name of the cross GCC binary    |
| CMAKE_TOOLCHAIN_FILE      |  Includes before everything else a compiler toolchain file      |
| CMAKE_BUILD_TYPE      |  Debug or Release (default ?)      |
| OST_BUNDLE      |  Specify the bundle name to build     |
| TOOLCHAIN_DIR      |  Specify a directory for the cross-gcc toolchain location    |


Example: `cmake -DTOOLCHAIN=riscv64-unknown-elf -DCMAKE_TOOLCHAIN_FILE=cmake/cross-gcc.cmake  -DCMAKE_BUILD_TYPE=Debug -DOST_BUNDLE=LONGAN_NANO ..`


# Hardware bundles

Here is a list of currently supported bundles. A bundle is a collection of electronics boards that are supported by official firmware builds.

Keep in mind that the official bundles proposed are not "cost-optimized". There are many rooms of improvements. The best bundles are marked with three stars "***": parts are easy to buy and the cost is minimal.

The goal of official bundles is to test the firmware on very different kind of hardware to make sure that the core firmware is highly portable.

The price indicated is purely informative.

## Sipeed Longan Nano (GD32VF103CBT6)

| Category | Maker |  Name  | Rounded Price |
|---|---|---|---|
|Main CPU board      | Sipeed       | Longan Nano    |  4???  |
| Audio              |              |                |  15??? |
| Memory storage     | Included SD card slot in Longan Nano |                |  -   |
| Battery management |              |                |  15??? |

### How to build

Tools for a Debian based distro

- sudo apt install crossbuild-essential-riscv64
- sudo apt install picolibc-riscv64-unknown-elf
  
mkdir build
cd build
cmake -DTOOLCHAIN=riscv64-unknown-elf -DCMAKE_TOOLCHAIN_FILE=cmake/cross-gcc.cmake  -DCMAKE_BUILD_TYPE=Debug -DOST_BUNDLE=LONGAN_NANO ..

Convert tools:

- riscv64-unknown-elf-objcopy -O binary your-file.elf your-file.hex
- riscv64-unknown-elf-objcopy -O ihex your-file.elf your-file.hex

### Wiring

TBD

## Arduino MKR Zero (SAMD21G18A)

| Category | Maker |  Name  | Rounded Price |
|---|---|---|---|
|Main CPU board      | Arduino        | MKR Zero    |  30???  |
| Audio              |              |                |  15??? |
| Memory storage     | Included SD card slot on board |                |  -   |
| Battery management | Included LiPo charger on board             |                |  - |
| Display | NewHaven  2.4" TFT        |    NHD-2.4-240320CF-BSXV            |  22??? |

### How to build

Install on Ubuntu : 
- sudo apt install gcc-arm-none-eabi
- sudo apt install picolibc-arm-none-eabi

cmake -DTOOLCHAIN=arm-none-eabi -DCMAKE_TOOLCHAIN_FILE=cmake/cross-gcc.cmake  -DCMAKE_BUILD_TYPE=Debug -DOST_BUNDLE=MKR_ZERO ..

### Wiring

TBD

# Mechanical and enclosure

Use existing enclosures, build your own using wood or 3D printing... we do not propose (yet) any standard package, sorry :(

# Future targets

- ESP32 (low cost and  high availability)
- RP2040

