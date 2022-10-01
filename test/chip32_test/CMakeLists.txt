cmake_minimum_required(VERSION 3.5)

project(chip_assembler_test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chip_assembler_test main.cpp test_parser.cpp ../../chip32/chip32_assembler.cpp)
target_include_directories(chip_assembler_test PRIVATE ../../chip32 ../)

install(TARGETS chip_assembler_test
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
