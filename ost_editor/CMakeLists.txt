cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME ost-editor)

project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
#find_package(SDL2 REQUIRED)

set(OST_EDITOR_ROOT  ${CMAKE_CURRENT_SOURCE_DIR})
set(OST_ROOT  ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(IMGUI_DIR ${OST_EDITOR_ROOT}/imgui)
set(IMGUI_NODE_EDITOR_DIR ${OST_EDITOR_ROOT}/imnodes)
set(IMGUI_KNOBS_DIR ${OST_EDITOR_ROOT}/imgui-knobs)
set(IMGUI_TEXT_EDIT_DIR ${OST_EDITOR_ROOT}/ImGuiColorTextEdit)

#=========================================================================================================================
# SOURCES
#=========================================================================================================================
set(PROJECT_SOURCES

    # ----------------  EDITOR FILES  ----------------
    ${OST_EDITOR_ROOT}/src/main.cpp
    ${OST_EDITOR_ROOT}/src/sdl_wrapper.cpp
    ${OST_EDITOR_ROOT}/src/imgui_memory_editor.h
    ${OST_EDITOR_ROOT}/src/sdl_wrapper.h
    ${OST_EDITOR_ROOT}/src/IconsFontAwesome5.h
    ${OST_EDITOR_ROOT}/src/code_editor.cpp
    ${OST_EDITOR_ROOT}/src/code_editor.h
    ${OST_EDITOR_ROOT}/src/console_window.cpp
    ${OST_EDITOR_ROOT}/src/console_window.h

    # ----------------  OST IMPLEMENTATION USING SDL  ----------------
    ${OST_EDITOR_ROOT}/src/ost_wrapper.cpp
    ${OST_EDITOR_ROOT}/src/ost_wrapper.h

    # ----------------  OST COMMON FILES  ----------------
    ${OST_ROOT}/src/picture.c
    ${OST_ROOT}/src/picture.h
    ${OST_ROOT}/src/debug.c
    ${OST_ROOT}/src/debug.h

    # ----------------  VIRTUAL MACHINE FILES  ----------------
    ${OST_EDITOR_ROOT}/src/chip32_assembler.h
    ${OST_EDITOR_ROOT}/src/chip32_assembler.cpp
    ${OST_ROOT}/common/chip32.h
    ${OST_ROOT}/common/chip32.cpp

    # ----------------  IMGUI  ----------------
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer.cpp

    # ----------------  IMGUI ADD ONS ----------------
    ${IMGUI_NODE_EDITOR_DIR}/imnodes.cpp
    ${IMGUI_NODE_EDITOR_DIR}/imnodes.h
    ${IMGUI_KNOBS_DIR}/imgui-knobs.cpp
    ${IMGUI_KNOBS_DIR}/imgui-knobs.h
    ${IMGUI_TEXT_EDIT_DIR}/TextEditor.cpp
    ${IMGUI_TEXT_EDIT_DIR}/TextEditor.h

)

set(OST_EDITOR_INCLUDES
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_NODE_EDITOR_DIR}
    ${IMGUI_TEXT_EDIT_DIR}
    ${OST_EDITOR_ROOT}/src
    ${IMGUI_KNOBS_DIR}
    ${OST_ROOT}/src
    ${OST_ROOT}/hal
    ${OST_ROOT}/common
)

#=========================================================================================================================
# SDL2
#=========================================================================================================================
include(ExternalProject)

ExternalProject_Add(sdl2_project
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 8c9beb0c873f6ca5efbd88f1ad2648bfc793b2ac #release-2.24.0
    PREFIX ${CMAKE_BINARY_DIR}/external
    INSTALL_COMMAND ""
)

set(SDL2_HEADERS
    ${CMAKE_BINARY_DIR}/external/src/sdl2_project/include
    ${CMAKE_BINARY_DIR}/external/src/sdl2_project/include/SDL2
)

set(SDL2_LIBS_DIR
    ${CMAKE_BINARY_DIR}/external/src/sdl2_project-build
)


#set(SDL2_HEADERS
#    /usr/local/include
#    /usr/local/include/SDL2
#    /usr/include/SDL2
#)

#set(SDL2_LIBS_DIR
#    /usr/local/lib
#)

set(SDL2_LIBS
    SDL2
)


#=========================================================================================================================
# FINAL EXE AND LINK OPTIONS
#=========================================================================================================================

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
add_dependencies(${PROJECT_NAME} sdl2_project)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBS} OpenGL::GL pthread dl)
target_link_directories(${PROJECT_NAME} PUBLIC ${SDL2_LIBS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_HEADERS} ${OST_EDITOR_INCLUDES})
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:OST_EDITOR_DEBUG>")
