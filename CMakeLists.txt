# ==================================================================================================
# OPEN STORY TELLER MAIN CMAKE BUILD FILE
# (see README.md for invokable options)
# Default target: TBD ?
# ==================================================================================================

cmake_minimum_required(VERSION 3.12)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)
set(PROJECT_NAME open-story-teller)

project(${PROJECT_NAME} LANGUAGES CXX C)

# ==================================================================================================
# BUILD TYPE
# ==================================================================================================
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# ==================================================================================================
# FINAL EXECUTABLE
# ==================================================================================================
#add_definitions(-DSDFAT_FILE_TYPE=2)



#add_subdirectory(lib)

#set(GCC_ARM_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/ports/atmel-samd21/ArduinoCore-samd/variants/mkrzero/linker_scripts/gcc/flash_without_bootloader.ld)
#set(GCC_ARM_LINKER_OPTIONS "-L${CMAKE_CURRENT_LIST_DIR}/ports/atmel-samd21/CMSIS/Lib -larm_cortexM0l_math")

#add_definitions(-DF_CPU=48000000L -DARDUINO=10813 -DARDUINO_SAMD_MKRZERO -DARDUINO_ARCH_SAMD -DUSE_ARDUINO_MKR_PIN_LAYOUT -D__SAMD21G18A__ -DUSB_VID=0x2341 -DUSB_PID=0x804f -DUSBCON "-DUSB_MANUFACTURER=\"Arduino LLC\"" "-DUSB_PRODUCT=\"Arduino MKRZero\"")
add_executable(
    ${PROJECT_NAME}
    src/main.c
    src/sdcard.c
    src/debug.c
    src/picture.c
    src/filesystem.c
    src/ff/ff.c
    src/ff/ffsystem.c
    src/ff/ff_stubs.c
)
include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ff
)

# ==================================================================================================
# BUNDLE LIBRARIES
# ==================================================================================================
if(${OST_BUNDLE} STREQUAL "LONGAN_NANO")
    message("=============  TARGET: LONGAN NANO  =============")

    add_definitions(-DOST_USE_FF_LIBRARY)

    # ------ CPU
    SET(CPU_TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cpu/gigadevice-gd32vf103)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cpu/gigadevice-gd32vf103/include.cmake)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpu/gigadevice-gd32vf103)

    # ------ PLATFORM
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/platform/sipeed-longan-nano)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform/sipeed-longan-nano)

    # Add middleware files
    target_sources(${PROJECT_NAME} PRIVATE library/ili9341.c)

    # link with gcc for software library such as float calculation (__udivdi3 ...)

    add_library(BUNDLE_LIBRARIES INTERFACE)
    target_link_libraries(BUNDLE_LIBRARIES INTERFACE "-Wl,--start-group" 
        gcc m c gigadevice-gd32vf103 sipeed-longan-nano
        "-Wl,--end-group")

    # SET(BUNDLE_LIBRARIES
    #     sipeed-longan-nano
    #     gigadevice-gd32vf103
    #     gcc m c c
    # )


else()
    message("============= DEFAULT TARGET: LINUX CONSOLE ============= ")
endif()


# ==================================================================================================
# FINAL LINK
# ==================================================================================================
target_link_libraries(${PROJECT_NAME} PUBLIC BUNDLE_LIBRARIES)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")





