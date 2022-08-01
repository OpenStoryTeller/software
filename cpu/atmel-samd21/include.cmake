
add_compile_definitions(__SAMD21G18A__)
add_compile_definitions(DONT_USE_CMSIS_INIT)
add_compile_definitions(F_CPU=48000000 )


set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m0plus -mthumb -Wl,--script=${CPU_TARGET_DIR}/linker/samd21j18.ld" CACHE INTERNAL "Linker options")

include_directories (
    ${CPU_TARGET_DIR}
    ${CPU_TARGET_DIR}/3rd-party
    ${CPU_TARGET_DIR}/3rd-party/pio
    ${CPU_TARGET_DIR}/3rd-party/instance
    ${CPU_TARGET_DIR}/3rd-party/component
    ${CPU_TARGET_DIR}/3rd-party/component
    ${CPU_TARGET_DIR}/3rd-party/CMSIS/Include
)

