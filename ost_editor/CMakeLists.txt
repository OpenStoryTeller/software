cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME ost-editor)

project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

set(OST_EDITOR_ROOT  ${CMAKE_CURRENT_SOURCE_DIR})
set(OST_ROOT  ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(IMGUI_DIR ${OST_EDITOR_ROOT}/imgui)
set(IMGUI_NODE_EDITOR_DIR ${OST_EDITOR_ROOT}/imnodes)
set(IMGUI_KNOBS_DIR ${OST_EDITOR_ROOT}/imgui-knobs)

#=========================================================================================================================
# SOURCES
#=========================================================================================================================
set(PROJECT_SOURCES

    # ----------------  EDITOR FILES  ----------------
    ${OST_EDITOR_ROOT}/src/main.cpp
    ${OST_EDITOR_ROOT}/src/chip8.c
    ${OST_EDITOR_ROOT}/src/chip8.h
    ${OST_EDITOR_ROOT}/src/sdl_wrapper.cpp
    ${OST_EDITOR_ROOT}/src/imgui_memory_editor.h
    ${OST_EDITOR_ROOT}/src/sdl_wrapper.h
    ${OST_EDITOR_ROOT}/src/IconsFontAwesome5.h

    # ----------------  OST IMPLEMENTATION USING SDL  ----------------
    ${OST_EDITOR_ROOT}/src/ost_wrapper.cpp
    ${OST_EDITOR_ROOT}/src/ost_wrapper.h

    # ----------------  OST COMMON FILES  ----------------
    ${OST_ROOT}/src/picture.c
    ${OST_ROOT}/src/picture.h
    ${OST_ROOT}/src/debug.c
    ${OST_ROOT}/src/debug.h

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

)

set(OST_EDITOR_INCLUDES
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_NODE_EDITOR_DIR}
    ${OST_EDITOR_ROOT}/src
    ${IMGUI_KNOBS_DIR}
    ${OST_ROOT}/src
    ${OST_ROOT}/hal
)

#=========================================================================================================================
# SDL2
#=========================================================================================================================
set(SDL2_HEADERS
    /usr/local/include
    /usr/local/include/SDL2
)

set(SDL2_LIBS_DIR
    /usr/local/lib
)

set(SDL2_LIBS
    SDL2
)


#=========================================================================================================================
# FINAL EXE AND LINK OPTIONS
#=========================================================================================================================

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBS} OpenGL::GL pthread)
target_link_directories(${PROJECT_NAME} PUBLIC ${SDL2_LIBS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_HEADERS} ${OST_EDITOR_INCLUDES})
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:OST_EDITOR_DEBUG>")
