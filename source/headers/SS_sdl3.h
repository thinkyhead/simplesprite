/*
 *  SS_sdl3.h
 *  Minimal bridge to build SimpleSprite against SDL 3.x.
 *  (c) 2026 Scott Lahteine.
 *
 *  SimpleSprite was written for SDL 1.2. The SDL 2.x port re-provided the few
 *  removed/renamed symbols in SS_sdl2.h. SDL3 removes/renames a few more, but
 *  most are already handled:
 *    - <SDL.h> etc. resolve via the sdl3_compat/ forwarding shims on the
 *      include path (SDL3 ships <SDL3/SDL.h>, <SDL3_image/SDL_image.h>, ...).
 *    - SDL_FreeSurface, SDL_HasColorKey, SDL_GetColorKey are STILL aliased in
 *      SDL3's SDL_oldnames.h, so we do NOT re-define them (would be a macro
 *      redefinition error).
 *
 *  Everything below is a single-site rename the engine makes that SDL3 dropped
 *  without a compat alias, kept here so the engine sources compile unchanged.
 *  The eventual SDL4(?) pass can replace/extend this file alone.
 */

#ifndef SS_SDL3_H
#define SS_SDL3_H

#include <SDL.h>

// --- Keyboard state -------------------------------------------------------
// SDL 1.2 SDL_GetKeyState -> SDL2/3 SDL_GetKeyboardState, scancode-indexed.
// In SDL3 it returns `const bool*`. The engine's keyState member is `const Uint8*`,
// so wrap and cast. The engine reads this via keys[SDL_SCANCODE_*] (see DS_Ship.cpp),
// which is the correct, in-bounds scancode index space in both SDL2 and SDL3.
// IMPORTANT: never index this array with an SDLK_* keycode constant — in SDL2/3
// those are 0x40000000|scancode (e.g. SDLK_UP = 0x40000052) and would read far
// out of bounds and crash.
inline const Uint8* SS_GetKeyState(int* numkeys) {
    return (const Uint8*)SDL_GetKeyboardState(numkeys);
}

// SDL 1.2 SDL_EnableKeyRepeat is gone in SDL2/3; text repeat is handled by the
// OS. Provide a no-op so call sites compile unchanged.
inline void SS_EnableKeyRepeat(int delay, int interval) {
    (void)delay; (void)interval;
}

// --- Key types ------------------------------------------------------------
// SDL2/3 renamed the key types. Keep the old names alive for the engine.
// `SDLKey` maps to SDL_Keycode (compatible). `SDLMod` we keep as a plain
// integer so assignments from SDL_Keysym.mod (an SDL_Keymod enum) and the
// KMOD_* constants both work without casts.
typedef SDL_Keycode SDLKey;
typedef Uint16     SDLMod;

// SDL 1.2 used KMOD_META for the Command/Windows key; SDL2/3 renamed it to
// KMOD_GUI. Provide the old name so the engine builds unchanged.
#ifndef KMOD_META
#define KMOD_META KMOD_GUI
#endif

// SDL 1.2 key-repeat defaults removed in SDL2/3; only used by our no-op
// SS_EnableKeyRepeat wrapper, so define the original values.
#ifndef SDL_DEFAULT_REPEAT_DELAY
#define SDL_DEFAULT_REPEAT_DELAY 500
#endif
#ifndef SDL_DEFAULT_REPEAT_INTERVAL
#define SDL_DEFAULT_REPEAT_INTERVAL 30
#endif

// --- Mouse-wheel button constants (GUI switch still references these) ------
// SDL2/3 removed SDL_BUTTON_WHEELUP/DOWN (wheel is now a dedicated
// SDL_EVENT_MOUSE_WHEEL event). The GUI still switches on them, so define the
// original 1.2 values to keep the cases compiling. They are effectively dead
// (the wheel is delivered via SDL_EVENT_MOUSE_WHEEL), but Plumbed through in
// SS_World::HandleEvents so zoom/scroll works in all three games.
#ifndef SDL_BUTTON_WHEELUP
#define SDL_BUTTON_WHEELUP 4
#endif
#ifndef SDL_BUTTON_WHEELDOWN
#define SDL_BUTTON_WHEELDOWN 5
#endif

// --- Cursor show/hide -----------------------------------------------------
// SDL3 removed SDL_DISABLE/SDL_ENABLE and split SDL_ShowCursor() into
// SDL_ShowCursor()/SDL_HideCursor(). Provide the 1.2 names + a thin wrapper.
#ifndef SDL_DISABLE
#define SDL_DISABLE 0
#endif
#ifndef SDL_ENABLE
#define SDL_ENABLE 1
#endif
inline int SS_ShowCursor(int toggle) {
    if (toggle == SDL_DISABLE) { SDL_HideCursor(); return 0; }
    SDL_ShowCursor(); return 0;
}
#define SDL_ShowCursor SS_ShowCursor

// --- Window grab ----------------------------------------------------------
// SDL2 SDL_SetWindowGrab -> SDL3 SDL_SetWindowMouseGrab (mouse only; there is
// no combined grab in SDL3 for our simple use). Keep the old name working.
#define SDL_SetWindowGrab(w, g) SDL_SetWindowMouseGrab((w), (g))

// --- Window creation ------------------------------------------------------
// SDL3 SDL_CreateWindow takes (title, w, h, flags) — no x/y. The engine calls
// it with 6 args (title, x, y, w, h, flags). Provide a compatible wrapper.
inline SDL_Window* SS_CreateWindow(const char* title, int, int, int w, int h, Uint32 flags) {
    return SDL_CreateWindow(title, w, h, flags);
}
#define SDL_CreateWindow SS_CreateWindow

#endif // SS_SDL3_H
