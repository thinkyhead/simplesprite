/*
 *  SS_sdl2.h
 *  Minimal bridge to build SimpleSprite against SDL 2.x.
 *  (c) 2026 Scott Lahteine.
 *
 *  SimpleSprite was written for SDL 1.2. SDL 2.x removed or renamed a few
 *  APIs. This header re-provides the small surface that the engine still
 *  needs so the bulk of the code compiles unchanged. The eventual SDL3 port
 *  can replace this file (or extend it) without touching the engine sources.
 */

#ifndef SS_SDL2_H
#define SS_SDL2_H

#include <SDL.h>

// SDL 1.2 let you read the current video surface as an SDL_Surface*.
// In SDL 2.x there is no such surface (pixels live in the GL context), so we
// keep a placeholder. The engine only ever used ss_screen->w / ss_screen->h,
// which we satisfy via the static accessors in SS_Game.
#ifndef SDL_VIDEO_W
#define SDL_VIDEO_W 0
#endif

// SDL 1.2 SDL_GetKeyState returned a process-wide array indexed by SDL_Keycode
// (small 1.2 values). SDL 2.x SDL_GetKeyboardState returns an array indexed by
// SDL_SCANCODE_*, which is SDL 2.x's native key-state space. The engine reads
// this array via keys[SDL_SCANCODE_*] (see DS_Ship.cpp), so we just hand back
// SDL's own scancode-indexed array. IMPORTANT: never index this array with an
// SDLK_* keycode constant — in SDL2 those are 0x40000000|scancode (e.g.
// SDLK_UP = 0x40000052) and would read far out of bounds and crash.
inline const Uint8* SS_GetKeyState(int* numkeys) {
    return SDL_GetKeyboardState(numkeys);
}

// SDL 1.2 SDL_EnableKeyRepeat is gone in SDL 2.x; text repeat is handled by
// the OS. Provide a no-op so call sites compile unchanged.
inline void SS_EnableKeyRepeat(int delay, int interval) {
    (void)delay; (void)interval;
}

// SDL 2.x renamed the key types. Keep the old names alive for the engine.
// `SDLKey` maps to SDL_Keycode (compatible). `SDLMod` we keep as a plain
// integer so assignments from SDL_Keysym.mod (an SDL_Keymod enum) and the
// KMOD_* constants both work without casts.
typedef SDL_Keycode SDLKey;
typedef Uint16     SDLMod;

// SDL 1.2 mouse-wheel button constants removed in SDL 2.x (wheel is now a
// dedicated SDL_MOUSEWHEEL event). The GUI still switches on them, so define
// the original 1.2 values to keep the cases compiling.
#ifndef SDL_BUTTON_WHEELUP
#define SDL_BUTTON_WHEELUP 4
#endif
#ifndef SDL_BUTTON_WHEELDOWN
#define SDL_BUTTON_WHEELDOWN 5
#endif

// SDL 1.2 used KMOD_META for the Command/Windows key; SDL 2.x renamed it to
// KMOD_GUI. Provide the old name so the engine builds unchanged.
#ifndef KMOD_META
#define KMOD_META KMOD_GUI
#endif

// SDL 1.2 key-repeat defaults removed in SDL 2.x; only used by our no-op
// SS_EnableKeyRepeat wrapper, so define the original values.
#ifndef SDL_DEFAULT_REPEAT_DELAY
#define SDL_DEFAULT_REPEAT_DELAY 500
#endif
#ifndef SDL_DEFAULT_REPEAT_INTERVAL
#define SDL_DEFAULT_REPEAT_INTERVAL 30
#endif

#endif // SS_SDL2_H
