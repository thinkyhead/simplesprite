# SimpleSprite

A C++ 2D sprite engine built on [SDL3](https://libsdl.org/) and OpenGL. Originally written in 2004, revived and ported to SDL3 in 2026.

## Description

SimpleSprite is a C++ framework based on SDL and OpenGL for creating games, screensavers, demos, and presentations. It loads and animates bitmap graphics, tests collisions between objects, and assigns atomic behaviors to game objects.

Originally inspired by [SpriteWorld](http://www.spriteworld.org/) (the library behind Escape Velocity), SimpleSprite implements sprite layers, action procs, and extends these concepts with OpenGL acceleration. The result is a clean, lightweight, flexible class library.

It uses SDL3, SDL3_image, and SDL3_mixer (the SDL3_mixer MIX_* API, not the old Mix_Chunk API) and is built via CMake with an optional Xcode project.

**Author:** Scott Lahteine (Thinkyhead)  
**License:** MIT  
**Repository:** `~/Projects/SDL/simplesprite/`

## Features

- **Image and Texture support** — loads PNG, JPEG, BMP, GIF via SDL3_image. OpenGL textures (potted, display-listed).
- **Sprite system** — `SS_Sprite` / `SS_Frame` with animation, collision masks, display lists.
- **Vector sprite system** — compiled geometry (lines, triangles, quads, circles, stars) via GLU tessellation.
- **Fonts, strings, and text blocks** — `SS_SFont` bitmap font system using SFont-format PNGs. Strings, edit fields, text layers.
- **Tiles and tile maps** — multi-layered tile environments with collision and object interaction.
- **User Interface** — `SS_GUI` / `SS_Gadget` widgets: buttons, checkboxes, sliders, dials, draggers, edit fields, menus, scrollbars, text input.
- **Sound Effects and Music** — `SS_Sound` / `SS_Music` using SDL3_mixer. WAV and AIFF sound effects, MP3/OGG music (depending on SDL3_mixer codec support).
- **Distributed AI** — Finite State Machine hooks on every sprite. AI Stacks for state transitions. Thousands of concurrent decision trees.
- **Inter-Object Messaging** — `SS_Broadcaster` / `SS_Listener` pattern for decoupled communication.
- **Collision System** — bitmask collision types, spatial band lists.
- **Particle System** — emitters, sprayers, droppers, streamers, fantails, tri-poly.
- **Layers** — z-ordered groupings with offset, scale, tint. Types: sprite, text, tile, GUI.
- **World / Game model** — `SS_World` (self-contained level) with `Run()` → `GetInput()` → `Process()` → `Render()` loop, owned by `SS_Game`. World stack for multi-state games.

## Build Dependencies

- **SDL3**, **SDL3_image**, **SDL3_mixer** (via Homebrew for development, or vendored)
- **OpenGL.framework** (macOS)
- **Cocoa.framework** (macOS, for bundle bootstrap)

## Building (macOS)

```bash
# Build the engine
cd ~/Projects/SDL/simplesprite
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Build your game (after engine)
cd ../mygame
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Xcode

A CMake-generated Xcode project is available:

```bash
cd ~/Projects/SDL/simplesprite/xcode-build
open SimpleSprite.xcodeproj
```

Select the `SimpleSpriteTest` scheme and click Run.

## Project Layout

```
simplesprite/
├── source/                          # Engine library sources
│   ├── headers/                     # All .h files
│   │   ├── sdl3_compat/             # Compatibility shims (#include <SDL.h> → <SDL3/SDL.h>)
│   │   ├── SS_sdl3.h                # SDL1.2 → SDL3 bridge (wraps renames + removed symbols)
│   │   ├── SS_Game.h, SS_World.h, … # Engine headers
│   │   └── SimpleSprite.h           # Umbrella header
│   ├── SS_Game.cpp, SS_World.cpp, … # Engine implementations
│   └── SS_Sound.cpp                 # Audio (rewritten for SDL3_mixer MIX_* API)
├── SimpleSprite Application/        # Sample app (Xcode project)
├── Documentation/                   # HTML class reference
└── CMakeLists.txt                   # CMake build
```

## Games Using SimpleSprite

- [DeepSpace Rifter](https://github.com/thinkyhead/DeepSpace) — space combat game
- [SolarFire](https://github.com/thinkyhead/SolarFire) — space shooter

## Port History

| Layer | Date | Notes |
|-------|------|-------|
| SDL 1.2 → SDL2 | 2026-07-17 | `SS_sdl2.h` bridge, fixed `SDLApplication` duplicate-class bug |
| SDL2 → SDL3 | 2026-07-18 | `SS_sdl3.h` bridge, `sdl3_compat` shims, audio rewrite for new MIX_* API |

## Documentation

Full HTML class reference is in the `Documentation/` directory. Open `Documentation/index.html` in a browser.
