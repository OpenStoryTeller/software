##########################################################################
# "THE ANY BEVERAGE-WARE LICENSE" (Revision 42 - based on beer-ware
# license):
# <dev@layer128.net> wrote this file. As long as you retain this notice
# you can do whatever you want with this stuff. If we meet some day, and
# you think this stuff is worth it, you can buy me a be(ve)er(age) in
# return. (I don't like beer much.)
#
# Matthias Kleemann
##########################################################################

cmake_minimum_required(VERSION 3.12)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE   ON)

set(PROJECT_NAME open-story-teller)
project(${PROJECT_NAME} LANGUAGES CXX C)

##########################################################################
# set build type
##########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)


# ==================================================================================================
# FINAL EXECUTABLE
# ==================================================================================================
#add_definitions(-DSDFAT_FILE_TYPE=2)


SET(CPU_TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ports/atmel-samd21)
include(ports/atmel-samd21/include.cmake)
add_subdirectory(ports/atmel-samd21)
#add_subdirectory(lib)

#set(GCC_ARM_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/ports/atmel-samd21/ArduinoCore-samd/variants/mkrzero/linker_scripts/gcc/flash_without_bootloader.ld)
#set(GCC_ARM_LINKER_OPTIONS "-L${CMAKE_CURRENT_LIST_DIR}/ports/atmel-samd21/CMSIS/Lib -larm_cortexM0l_math")

#add_definitions(-DF_CPU=48000000L -DARDUINO=10813 -DARDUINO_SAMD_MKRZERO -DARDUINO_ARCH_SAMD -DUSE_ARDUINO_MKR_PIN_LAYOUT -D__SAMD21G18A__ -DUSB_VID=0x2341 -DUSB_PID=0x804f -DUSBCON "-DUSB_MANUFACTURER=\"Arduino LLC\"" "-DUSB_PRODUCT=\"Arduino MKRZero\"")
add_executable(
    ${PROJECT_NAME}
    src/main.c
    src/sdcard.c
    src/debug.c
    src/filesystem.c
    src/ff/ff.c
    src/ff/ffsystem.c
    src/ff/ff_stubs.c
)
include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ff
)

target_link_libraries(${PROJECT_NAME} PUBLIC atmel-samd21 gcc)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

