/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Game.h
 *
 *  $Id: SS_Game.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_GAME_H__
#define __SS_GAME_H__

#include <math.h>

// Global video dimensions (defined in SS_Game.cpp). Declared up-front so the
// inline accessors below can reference them regardless of include order.
extern int SS_VIDEO_W, SS_VIDEO_H;

#include <SDL.h>
#include "SS_Types.h"

// SDL 2.x bridge (SS_GetKeyState, SS_EnableKeyRepeat, SDL_VIDEO_W alias)
#include "SS_sdl2.h"

class SS_Game
{
    private:
        static SDL_Surface          *ss_screen;     // SDL 1.2 remnant; unused under SDL2 (see ss_window)
        static SDL_Window           *ss_window;     // SDL 2.x window handle
        static SDL_GLContext        ss_glcontext;   // SDL 2.x OpenGL context
        static float                SS_cos[65536];
        static float                SS_sin[65536];
        bool                        bQuit;                      // Game should quit?

    public:
        static SS_SFont             *tinyFont, *smallFont, *mediumFont, *largeFont;

        SS_World                    *world;                     // The active world
        Uint16                      worldCount;                 // Number of active worlds
        SS_World                    *worlds[SS_MAX_WORLDS];     // The active world stack

                                    SS_Game();
        virtual                     ~SS_Game();

        void                        InitTrigonometry();

        static inline float AngleOfXY(const float &x, const float &y)
        {
            // Guard atan2(0,0): Apple's libm returns NaN there, which propagates
            // into SS_ROTINDEX and corrupts rotation. A zero vector has no angle,
            // so report 0 (matching the original SDL1.2 / Carbon behavior).
            if (x == 0.0f && y == 0.0f) return 0.0f;
            float   dx = (x == 0.0f) ? 0.00001f : x;
            return (dx < 0.0f) ? (DEG(atan(y / dx)) + 270.0f) : (DEG(atan(y / dx)) + 90.0f);
        }

        static inline float Hypotenuse(float &x, float &y) { return sqrt(x * x + y * y); }

        static inline float LineLength(float &x1, float &y1, float &x2, float &y2)
        {
            float dx = x2 - x1, dy = y2 - y1;
            return Hypotenuse(dx, dy);
        }

        static inline void Normalize(float &xp, float &yp, float r=1.0f)
        {
            float mf = MAX(ABS(xp), ABS(yp));
            if (mf != 0.0f) xp /= mf, yp /= mf;
            xp *= r; yp *= r;
        }

        static inline void Rotate(float &xp, float &yp, float rot)
        {
            Uint16  i = SS_ROTINDEX(rot);
            float   rcos = Cos(i);
            float   rsin = Sin(i);
            float   rx = (xp * rcos - yp * rsin);
            float   ry = (yp * rcos + xp * rsin);
            xp = rx; yp = ry;
        }

        static inline void Rotate(SS_Point &point, float rot) {
            Rotate(point, SS_ROTINDEX(rot));
        }

        static inline void Rotate(float &xp, float &yp, Uint16 i)
        {
            float   rcos = Cos(i);
            float   rsin = Sin(i);
            float   rx = (xp * rcos - yp * rsin);
            float   ry = (yp * rcos + xp * rsin);
            xp = rx; yp = ry;
        }

        static inline void Rotate(SS_Point &point, Uint16 i)
        {
            Rotate(point.x, point.y, (Uint16)i);
            point.i += i;
            point.r = SS_ROTANGLE(point.i);
        }

        static inline double        Cos(Uint16 i)   { return SS_cos[i]; }
        static inline double        Sin(Uint16 i)   { return SS_sin[i]; }

        void                        InitScreen();
        static inline SDL_Surface*  TheScreen() { return ss_screen; }
        static inline SDL_Window*   TheWindow() { return ss_window; }
        static inline int           ScreenWidth() { return SS_VIDEO_W; }
        static inline int           ScreenHeight() { return SS_VIDEO_H; }
        static void                 SyncVblank(long sync);

        void                        PushWorld(SS_World *w);
        void                        PopWorld();
        inline SS_World*            SetWorld(SS_World *w) { return world = w; }

        virtual void                Run() = 0;

        void                        SaveGame();
        void                        LoadGame();
        void                        LoadConfig();
        void                        Quit()          { bQuit = true; }
        bool                        IsQuitting()    { return bQuit; }

    private:
        void                Init();
};

#endif
