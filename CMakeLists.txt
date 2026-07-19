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

# ----- SDL3 (headers: vendored sdl3_compat; linking: Homebrew) ---------------
# Header shims live in source/headers/sdl3_compat so bare #include <SDL.h> etc.
# resolve to <SDL3/SDL.h>. The vendored SDL3 include dir adds <SDL3/...> and
# <SDL3_image/...> paths.
set(SDL3_VENDORED "${CMAKE_CURRENT_SOURCE_DIR}/../_LIBS/SDL3")
set(SDL3_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/headers/sdl3_compat"
    "${SDL3_VENDORED}/include")

# For development, link against Homebrew-installed dylibs (all transitive deps
# resolve via /opt/homebrew/lib). For distribution, bundle dylibs manually.
find_library(SDL3_LIB_REAL NAMES SDL3 SDL3.0 SDL3-3.0
    PATHS /opt/homebrew/lib /opt/homebrew/opt/sdl3/lib NO_DEFAULT_PATH)
find_library(SDL3_IMAGE_LIB_REAL NAMES SDL3_image SDL3_image.0
    PATHS /opt/homebrew/lib /opt/homebrew/opt/sdl3_image/lib NO_DEFAULT_PATH)
find_library(SDL3_MIXER_LIB_REAL NAMES SDL3_mixer SDL3_mixer.0
    PATHS /opt/homebrew/lib /opt/homebrew/opt/sdl3_mixer/lib NO_DEFAULT_PATH)

set(SDL3_LINK_LIBS
    "${SDL3_LIB_REAL}" "${SDL3_IMAGE_LIB_REAL}" "${SDL3_MIXER_LIB_REAL}"
    "-Wl,-rpath,/opt/homebrew/lib")

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
    target_compile_definitions(SimpleSprite PRIVATE SS_ASSERT_ON=1 GL_SILENCE_DEPRECATION=1)
endif()

# SDL2 framework headers are reached three ways:
#  - -F for framework-style <SDL2/SDL.h> (and the linker)
#  - -I on each framework's Headers dir so bare <SDL.h> / <SDL_image.h> / <SDL_mixer.h> resolve
target_include_directories(SimpleSprite PRIVATE
    ${SDL3_INCLUDE_DIRS})

# Link the three vendored SDL3 dylibs; transitive codec deps resolve via @rpath.
target_link_libraries(SimpleSprite PRIVATE
    ${SDL3_LINK_LIBS})

# OpenGL (system framework, still ships on macOS)
target_link_libraries(SimpleSprite PUBLIC
    "-framework OpenGL")

# Quiet the deprecation noise from SDL_SetAlpha etc. (kept for minimal churn)
target_compile_definitions(SimpleSprite PRIVATE
    SDL_DISABLE_DEPRECATED=0
    SDL_ENABLE_OLD_NAMES=1
    GL_SILENCE_DEPRECATION=1)

# Legacy code freely narrows double->float in vector initializer lists; the
# values are well-defined so silence the C++11 narrowing error.
target_compile_options(SimpleSprite PRIVATE
    -Wno-c++11-narrowing)

# ----- Install (optional) -----------------------------------------------------
install(TARGETS SimpleSprite ARCHIVE DESTINATION lib)
install(DIRECTORY source/headers/ DESTINATION include/SimpleSprite FILES_MATCHING PATTERN "*.h")

# ----- Sample App (macOS bundle, Xcode-friendly) -----------------------------
add_executable(SimpleSpriteTest MACOSX_BUNDLE
    "SimpleSprite Application/source/My_Game.h"
    "SimpleSprite Application/source/My_Game.cpp"
    "SimpleSprite Application/source/main.mm"
)
target_sources(SimpleSpriteTest PRIVATE
    "source/headers/SimpleSprite.h"
)
target_include_directories(SimpleSpriteTest PRIVATE
    "source/headers"
    "source/headers/sdl3_compat"
    "SimpleSprite Application/source"
    ${SDL3_INCLUDE_DIRS}
)
target_compile_definitions(SimpleSpriteTest PRIVATE
    SDL_ENABLE_OLD_NAMES=1 SDL_MAIN_HANDLED=1 GL_SILENCE_DEPRECATION=1)
target_compile_options(SimpleSpriteTest PRIVATE -Wno-c++11-narrowing -fobjc-arc)

# Link the engine + SDL3 libraries
target_link_libraries(SimpleSpriteTest PRIVATE
    SimpleSprite
    ${SDL3_LINK_LIBS}
    "-framework OpenGL"
    "-framework Cocoa"
)
set_target_properties(SimpleSpriteTest PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.thinkyhead.ss-sdl3-test"
    MACOSX_BUNDLE_BUNDLE_NAME "SimpleSprite SDL3 Test"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
# Copy font data into the bundle's Resources
add_custom_command(TARGET SimpleSpriteTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/SimpleSprite Application/data"
        "$<TARGET_BUNDLE_CONTENT_DIR:SimpleSpriteTest>/Resources/data"
    COMMENT "Copying font data into app bundle")
