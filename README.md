# Open Story Teller (OST)

Open Story Teller is an Open Source project to provide guidelines and software to build your own story teller box. 

The main goal is to *not* make electronics boards but instead buy or reuse existing ones. This will allow you to easily repair your device with spare parts.

We propose a set of parts and firmware that is working well togather but your are free to custom everything to fit your needs.

# Firmware/software

The firmware is highly configurable and highly portable. To achieve that, it is split in multiple parts:
- The core source file, which is common to every target
- the ports, dedicated to an embedded MCU and board
- The tests, to easily test part of source on a standard PC
- A desktop/mobile implementation

The core is written in pure C, targets implementations may add other languages and libraries (QML/C++/python ...).

# Hardware bundles

Here is a list of currently supported bundles. A bundle is a collection of electronics boards that are supported by official firmware builds.

Keep in mind that the official bundles proposed are not "cost-optimized". There are many rooms of improvements. The best bundles are marked with three stars "***": parts are easy to buy and the cost is minimal.

The goal of official bundles is to test the firmware on very different kind of hardware to make sure that the core firmware is highly portable.

The price indicated is purely informative.

## Sipeed Longan Nano (GD32VF103CBT6)

| Category | Maker |  Name  | Rounded Price |
|---|---|---|---|
|Main CPU board      | Sipeed       | Longan Nano    |  4€  |
| Audio              |              |                |  10€ |
| Memory storage     | Included SD card slot in Longan Nano |                |  -   |
| Battery management |              |                |  10€ |

Link to specific bundle page information : TODO

### How to build

Tools for a Debian based distro

- sudo apt install crossbuild-essential-riscv64
- sudo apt install picolibc-riscv64-unknown-elf
  
mkdir build
cd build
cmake -DTOOLCHAIN=riscv64-unknown-elf -DCMAKE_TOOLCHAIN_FILE=cmake/cross-gcc.cmake  -DCMAKE_BUILD_TYPE=Debug -DOST_BUNDLE=LONGAN_NANO ..


Compiler flags: 
-std=gnu11 -Wall -march=rv32imac -mabi=ilp32 -mcmodel=medlow -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Og -g2 -ggdb2 -DPLATFORMIO=60103 -DUSE_STDPERIPH_DRIVER -DHXTAL_VALUE=8000000U -D__PLATFORMIO_BUILD_DEBUG__ 

Linker flags

riscv-nuclei-elf-gcc -o .pio/build/sipeed-longan-nano/firmware.elf -T /home/anthony/.platformio/packages/framework-gd32vf103-sdk/RISCV/env_Eclipse/GD32VF103xB.lds -march=rv32imac -mabi=ilp32 -mcmodel=medlow -nostartfiles -Xlinker --gc-sections --specs=nano.specs -Og -g2 -ggdb2 .pio/build/sipeed-longan-nano/src/audio.o .pio/build/sipeed-longan-nano/src/i2s.o .pio/build/sipeed-longan-nano/src/ili9341.o .pio/build/sipeed-longan-nano/src/main.o .pio/build/sipeed-longan-nano/src/sdcard.o .pio/build/sipeed-longan-nano/src/spi.o .pio/build/sipeed-longan-nano/src/spi0.o .pio/build/sipeed-longan-nano/src/systick.o -L.pio/build/sipeed-longan-nano -Wl,--start-group .pio/build/sipeed-longan-nano/libef6/libff.a .pio/build/sipeed-longan-nano/libstandard_peripheral.a .pio/build/sipeed-longan-nano/libRISCV.a -lm -lc -lc -Wl,--end-group

Convert tools:

riscv-nuclei-elf-objcopy -O binary .pio/build/sipeed-longan-nano/firmware.elf .pio/build/sipeed-longan-nano/firmware.bin
riscv-nuclei-elf-objcopy -O ihex .pio/build/sipeed-longan-nano/firmware.elf .pio/build/sipeed-longan-nano/firmware.hex



# Mechanical and enclosure

Use existing enclosures, build your own using wood or 3D printing... we do not propose (yet) any standard package.

# Future targets

- ESP32 (low cost and  high availability)
- RP2040

