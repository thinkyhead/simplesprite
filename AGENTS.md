# SimpleSprite — Agent Context

## Overview

SimpleSprite is an OpenGL 2D sprite engine built on SDL (1.2.x). Copyright 2004 Scott Lahteine, MIT licensed. It provides a complete 2D game engine framework: sprite management, layers, tile maps, vector rendering, GUI widgets, collisions, sound, particles, file I/O, and a world/game loop architecture.

## Source Layout

```
simplesprite/
├── source/                          # Core SimpleSprite library
│   ├── headers/                     # All .h header files
│   │   ├── SimpleSprite.h           # Convenience umbrella header
│   │   ├── SS_Types.h              # Types, enums, macros, inlines
│   │   ├── SS_Config.h             # Build config (debug, threads, audio, joystick)
│   │   ├── SS_Game.h               # Game class — screen, worlds, trig
│   │   ├── SS_World.h              # World — layer stack, events, loop
│   │   ├── SS_Layer.h              # Layer — items, visibility, transforms
│   │   ├── SS_LayerItem.h          # Base item class for all objects
│   │   ├── SS_Sprite.h             # Sprite — frames, animation, rendering
│   │   ├── SS_Frame.h              # Frame — textures, collision masks
│   │   ├── SS_Vectors.h            # Vector sprite system
│   │   ├── SS_Particles.h          # Particle/emitter framework
│   │   ├── SS_Collisions.h         # Collider + CollisionManager
│   │   ├── SS_Math.h               # Math utilities
│   │   ├── SS_Files.h              # File I/O, Folder, FlatFile, DataToken
│   │   ├── SS_SFont.h              # SFont-based font system
│   │   ├── SS_Sound.h              # Audio through SDL_mixer
│   │   ├── SS_GUI.h                # GUI widget system
│   │   ├── SS_Messages.h           # Messaging/broadcaster system
│   │   ├── SS_Templates.h          # Generic templates (TList, TArray)
│   │   ├── SS_ItemGroup.h          # Grouped items
│   │   ├── SS_ItemList.h           # Item list collections
│   │   ├── SS_Tiles.h              # Tile map + tile layer system
│   │   ├── SS_RefCounter.h         # Reference counting
│   │   ├── SS_Deformers.h          # Sprite deformation
│   │   ├── SS_AI.h                 # AI framework
│   │   ├── SS_Utilities.h          # Utility functions
│   │   └── SS_LayerItem.h          # Base item class
│   ├── SS_Game.cpp                 # Game init, screen, trig, world stack
│   ├── SS_World.cpp                # World run loop, events, process/render
│   ├── SS_Layer.cpp                # Layer item management
│   ├── SS_LayerItem.cpp            # Base item implementation
│   ├── SS_Sprite.cpp               # Sprite rendering
│   ├── SS_Frame.cpp                # Texture loading, display lists
│   ├── SS_Files.cpp                # File I/O implementation
│   ├── SS_SFont.cpp                # Font rendering
│   ├── SS_Sound.cpp                # SDL_mixer wrapper
│   ├── SS_GUI.cpp                  # GUI widgets
│   ├── SS_Messages.cpp             # Message system
│   ├── SS_Collisions.cpp           # Collision detection
│   ├── SS_Vectors.cpp              # Vector rendering
│   ├── SS_Math.cpp                 # Math funcs
│   ├── SS_Utilities.cpp            # Utility funcs
│   ├── SS_ItemGroup.cpp            # Item groups
│   ├── SS_ItemList.cpp             # Item list collection
│   ├── SS_Tiles.cpp                # Tile system
│   ├── SS_Particles.cpp            # Particles
│   ├── SS_Deformers.cpp            # Deformation
│   └── SS_AI.cpp                   # AI framework
├── SimpleSprite Application/       # Sample application using SimpleSprite
│   ├── source/
│   │   ├── main.cpp                # App entry point
│   │   ├── My_Game.cpp/.h          # Game subclass
│   │   └── SDLMain.m/.h            # macOS SDL glue
│   ├── data/font/                  # Bitmap fonts
│   └── SimpleSpriteApp.xcodeproj   # Xcode project
├── Documentation/                  # HTML docs for the library
├── xcode/                          # Old Xcode project (modern)
├── xcode-2007/                     # Very old Xcode project
└── LICENSE                         # MIT
```

## Architecture

### Design Pattern: Game → World → Layer → Item
- **SS_Game** — top-level controller. Owns the SDL screen, trig tables, a world stack. Pure virtual `Run()` must be subclassed.
- **SS_World** — a self-contained game mode/level. Contains layers, manages events, runs process/animate/render loop, collision detection. Supports threaded operation.
- **SS_Layer** — a z-ordered grouping of items with its own offset, scale, tint. Types: plain, sprite, text, tile, GUI.
- **SS_LayerItem** — base class for all renderable objects. Provides position, rotation, scale, visibility, motion functions.

### Key Subsystems
- **Sprite System** — `SS_Sprite` holds multiple `SS_Frame` objects. Frames load PNG surfaces, create OpenGL textures (POT) and display lists. Collision masks per frame.
- **Vector System** — `SS_VectorSprite` / `SS_VectorFrame` / `SS_VectorUnit`. Renders compiled geometry (lines, triangles, quads, circles, stars, etc.) via display lists. Uses GLU tessellation for non-convex polygons.
- **Font System** — `SS_SFont` uses the SFont bitmap format. Each character is a separate OpenGL texture. Supports `SS_String`, `SS_EditString`, `SS_TextLayer`.
- **GUI System** — `SS_GUI` is a layer with gadget management. Widgets: button, checkbox, radio, slider, dragger, dial, edit string, scrollbar, menu, menubar, text input.
- **Collision System** — `SS_Collider` hierarchy with bitmask collision types (COLL_LASER, COLL_BULLET, etc.). Spatial band-based collision list. `SS_CollisionManager::RunCollisionTest()` iterates lists.
- **Particle System** — `SS_Emitter`, `SS_Sprayer`, `SS_Dropper`, `SS_Streamer`, `SS_Fantail`, `SS_TriPoly`. Interval-based emission with chaos factor.
- **Audio** — `SS_Sound` wraps SDL_mixer. Supports sound effects and music (MP3 via smpeg). Retain/release refcounting.
- **Messaging** — `SS_Broadcaster`/`SS_Listener` pattern for decoupled inter-object communication.
- **File I/O** — `SS_File`, `SS_Folder` with directory stack, `SS_FlatFile` for config/data files with context/token system.

### Rendering Pipeline
Uses OpenGL 1.x fixed-function pipeline (no shaders). Texture tinting via `GL_COLOR_MATERIAL` + `GL_LIGHTING` + `GL_LIGHT0`. Each frame renders via display list. Layer prepares modelview matrix with offset/scale/rotation. World runs `PreRender()` → layers → `PostRender()`.

### World Loop
`SS_World::Run()`:
1. `GetInput()` — polls SDL events
2. `Process()` — runs layer Process, then Animate, then Collision
3. `Render()` — clears, runs PreRender, layers, PostRender
4. `PostProcess()` — overridable per-world
5. Tracks FPS

### Build Dependencies
- SDL 3.x (vendored dylibs + headers at `~/Projects/SDL/_LIBS/SDL3/`)
- SDL3_image (PNG loading, vendored alongside SDL3)
- SDL3_mixer (audio, vendored alongside SDL3 — note: 3.x is a new MIX_* API,
  not the old Mix_Chunk API; `SS_Sound` was rewritten for it)
- OpenGL.framework
- Cocoa.framework (macOS SDLMain, now the thin `SDL_MAIN_HANDLED` bootstrap)

### Rendering Pipeline
Uses OpenGL 1.x fixed-function pipeline (no shaders). Texture tinting is done via
the texture environment (`GL_MODULATE`) combined with `glColor` — **not** `GL_LIGHTING`.
`SS_Game::InitScreen` explicitly disables `GL_LIGHTING`/`GL_LIGHT0`/`GL_COLOR_MATERIAL`
(news: enabling them with no normals set erased translucent textured sprites).
Each frame renders via display list. Layer prepares modelview matrix with
offset/scale/rotation. World runs `PreRender()` → layers → `PostRender()`.

## Build Configuration (SS_Config.h)
- `SS_DEBUG` — debug output level (0-2)
- `SS_AUDIO_ENABLE` — audio on/off
- `SS_JOYSTICK_ENABLE` — joystick support
- `SS_MAX_WORLDS` — world stack depth (10)
- `SS_COLLISION_LISTS` — spatial collision list count (100)
- `SS_COLLISION_BAND_SIZE` — collision band width (200)

## Video Configuration (SS_Game.cpp)
- Default: 1024x768 windowed, VSync off
- Config file `ssgame.cfg` stored in app folder

## Current State
- **SDL 3.x port complete** (2026-07-18). The engine builds `libSimpleSprite.a`
  and links against vendored SDL3/SDL3_image/SDL3_mixer from
  `~/Projects/SDL/_LIBS/SDL3/` (no Homebrew at build time). The SDL1.2→SDL3 seam is
  the bridge in `source/headers/SS_sdl3.h`; `<SDL.h>` etc. resolve via the
  `source/headers/sdl3_compat/` forwarding-include shims. `SDL_ENABLE_OLD_NAMES=1`
  is defined so the renamed-but-aliased SDL3 symbols (SDL_KEYDOWN, SDL_mutex,
  KMOD_NONE, …) resolve — SDL3 *poisons* those names by default.
  Games that use it: `../solarfire`, `../deepspace`. See `~/wiki/llm-wiki/sdl-games-revival.md`.
- **Audio rewrite:** SDL3_mixer 3.x removed the old `Mix_Chunk` API entirely (it is a
  new MIX_Mixer/MIX_Audio/MIX_Track model). `SS_Sound`/`SS_Music` were rewritten onto
  the new `MIX_*` API in `source/SS_Sound.cpp` + `source/headers/SS_Sound.h`; the old
  integer-channel API (Stop/Pause/SetVolume/SetPanning/…) is preserved via a channel→
  track registry. Future-feature hooks (3D position, fades, frequency ratio) are
  commented in that file.
- During the revival a GL_LIGHTING bug in `SS_Game::InitScreen` was fixed (sprites
  were rendering invisible); rebuild `libSimpleSprite.a` after any engine change.
- **Headless note:** under `SDL_VIDEODRIVER=dummy`/`SDL_AUDIODRIVER=dummy` the engine
  *hangs* in `SDL_GL_CreateContext` (dummy GL) and in `MIX_CreateMixer` (dummy audio),
  because the sandbox has no display/audio device. That is an environment limit, not a
  code bug — verify rendering/audio on a real display.
