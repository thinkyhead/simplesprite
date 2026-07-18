cmake_minimum_required(VERSION 3.20)
project(SimpleSprite CXX)

# ----------------------------------------------------------------------------
# SimpleSprite — OpenGL 2D sprite engine (orig. SDL 1.2, ported to SDL 2.x)
# Builds a static library (libSimpleSprite.a).
#
# Dev build defaults to the host architecture for speed.
# Release builds can pass -DSS_UNIVERSAL=ON (or -DCMAKE_OSX_ARCHITECTURES)
# to produce an arm64 + x86_64 universal binary.
# ----------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pin to the system clang to avoid any MacPorts gcc override on PATH.
if(APPLE AND NOT DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER   "/usr/bin/clang")
endif()
if(APPLE AND NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif()

# ----- Architecture -----------------------------------------------------------
# Dev: current arch only. Release: pass -DSS_UNIVERSAL=ON for fat binary.
if(SS_UNIVERSAL)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)
else()
    # Leave CMAKE_OSX_ARCHITECTURES unset -> CMake uses the host arch.
endif()

# ----- SDL2 (from _LIBS, kept alongside the project) -------------------------
set(SDL_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../_LIBS/SDL/Frameworks")
get_filename_component(SDL_LIBS_DIR "${SDL_LIBS_DIR}" ABSOLUTE)

set(SDL2_FRAMEWORK       "${SDL_LIBS_DIR}/SDL2.framework")
set(SDL2_IMAGE_FRAMEWORK "${SDL_LIBS_DIR}/SDL2_image.framework")
set(SDL2_MIXER_FRAMEWORK "${SDL_LIBS_DIR}/SDL2_mixer.framework")

foreach(fw ${SDL2_FRAMEWORK} ${SDL2_IMAGE_FRAMEWORK} ${SDL2_MIXER_FRAMEWORK})
    if(NOT EXISTS "${fw}")
        message(FATAL_ERROR "Missing SDL framework: ${fw}\n"
            "Install SDL2 into ${SDL_LIBS_DIR} or adjust SDL_LIBS_DIR.")
    endif()
endforeach()

# ----- Sources ----------------------------------------------------------------
file(GLOB SS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

add_library(SimpleSprite STATIC ${SS_SOURCES})

target_include_directories(SimpleSprite PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/source/headers")

# ----- Asserts for testing (Debug only) -------------------------------------
# SS_ASSERT_ON enables SS_ASSERT / SS_ASSERT_FINITE in SS_Config.h — used to
# catch ref-count underflow (dangling pointers) and NaN angles during bring-up.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(SimpleSprite PRIVATE SS_ASSERT_ON=1)
endif()

# SDL2 framework headers are reached three ways:
#  - -F for framework-style <SDL2/SDL.h> (and the linker)
#  - -I on each framework's Headers dir so bare <SDL.h> / <SDL_image.h> / <SDL_mixer.h> resolve
target_include_directories(SimpleSprite PRIVATE
    "${SDL2_FRAMEWORK}/Headers"
    "${SDL2_IMAGE_FRAMEWORK}/Headers"
    "${SDL2_MIXER_FRAMEWORK}/Headers")

# Framework search path + link the three SDL2 frameworks.
target_compile_options(SimpleSprite PRIVATE
    "-F${SDL_LIBS_DIR}")
target_link_options(SimpleSprite PRIVATE
    "-F${SDL_LIBS_DIR}"
    "-Wl,-framework,SDL2"
    "-Wl,-framework,SDL2_image"
    "-Wl,-framework,SDL2_mixer")

# OpenGL (system framework, still ships on macOS)
target_link_libraries(SimpleSprite PUBLIC
    "-framework OpenGL")

# Make SDL/SDL_image/SDL_mixer headers reachable as <SDL.h>, <SDL_image.h>, ...
set_target_properties(SimpleSprite PROPERTIES
    FRAMEWORK_SEARCH_PATHS "${SDL_LIBS_DIR}")

# Quiet the deprecation noise from SDL_SetAlpha etc. (kept for minimal churn)
target_compile_definitions(SimpleSprite PRIVATE
    SDL_DISABLE_DEPRECATED=0)

# Legacy code freely narrows double->float in vector initializer lists; the
# values are well-defined so silence the C++11 narrowing error.
target_compile_options(SimpleSprite PRIVATE
    -Wno-c++11-narrowing)

# ----- Install (optional) -----------------------------------------------------
install(TARGETS SimpleSprite ARCHIVE DESTINATION lib)
install(DIRECTORY source/headers/ DESTINATION include/SimpleSprite FILES_MATCHING PATTERN "*.h")
