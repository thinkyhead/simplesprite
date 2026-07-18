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

# ----- SDL3 (vendored into _LIBS, no Homebrew at build time) ---------------
# SDL 1.2/2.x -> 3.x port. SDL3 ships headers under <SDL3/SDL.h>,
# <SDL3_image/SDL_image.h>, <SDL3_mixer/SDL_mixer.h>; the engine's bare
# <SDL.h> etc. are satisfied by the sdl3_compat/ forwarding shims on the
# include path (see source/headers/sdl3_compat/).
set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../_LIBS/SDL3")
get_filename_component(SDL3_DIR "${SDL3_DIR}" ABSOLUTE)

# Header search: the compat shims live in source/headers/sdl3_compat so the
# engine's #include <SDL.h> resolves to <SDL3/SDL.h>. We ALSO add the vendored
# include root so <SDL3/...> and <SDL3_image/...> resolve directly.
set(SDL3_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/headers/sdl3_compat"
    "${SDL3_DIR}/include")

# Vendored dylibs + link libs. Each dylib is re-id'd to @rpath/<name>, so we
# add an rpath to the vendored lib dir and link the three SDL3 libs (the codec
# transitive deps resolve via @rpath at load time).
set(SDL3_LIB_DIR "${SDL3_DIR}/lib")
set(SDL3_LINK_LIBS
    "${SDL3_LIB_DIR}/libSDL3.dylib"
    "${SDL3_LIB_DIR}/libSDL3_image.dylib"
    "${SDL3_LIB_DIR}/libSDL3_mixer.dylib")

foreach(lib ${SDL3_LINK_LIBS})
    if(NOT EXISTS "${lib}")
        message(FATAL_ERROR "Missing SDL3 library: ${lib}\n"
            "Expected vendored SDL3 at ${SDL3_DIR}. Run the vendoring step.")
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
    ${SDL3_INCLUDE_DIRS})

# Link the three vendored SDL3 dylibs; transitive codec deps resolve via @rpath.
target_link_libraries(SimpleSprite PRIVATE
    ${SDL3_LINK_LIBS}
    "-Wl,-rpath,${SDL3_LIB_DIR}")

# OpenGL (system framework, still ships on macOS)
target_link_libraries(SimpleSprite PUBLIC
    "-framework OpenGL")

# Quiet the deprecation noise from SDL_SetAlpha etc. (kept for minimal churn)
target_compile_definitions(SimpleSprite PRIVATE
    SDL_DISABLE_DEPRECATED=0
    SDL_ENABLE_OLD_NAMES=1)

# Legacy code freely narrows double->float in vector initializer lists; the
# values are well-defined so silence the C++11 narrowing error.
target_compile_options(SimpleSprite PRIVATE
    -Wno-c++11-narrowing)

# ----- Install (optional) -----------------------------------------------------
install(TARGETS SimpleSprite ARCHIVE DESTINATION lib)
install(DIRECTORY source/headers/ DESTINATION include/SimpleSprite FILES_MATCHING PATTERN "*.h")
