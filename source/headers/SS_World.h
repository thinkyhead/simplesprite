/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_World.h
 *
 *  $Id: SS_World.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_WORLD_H__
#define __SS_WORLD_H__

#include "SS_ItemList.h"
#include "SS_Collisions.h"
#include "SS_Messages.h"

#include "SS_Types.h"

#include <SDL_thread.h>
#include <SDL.h>

typedef bool (*worldEventProc)(SS_World *w, SDL_Event *e);
typedef void (*worldProc)(SS_World *w);

#pragma mark -
//--------------------------------------------------------------
// SS_World
//
class SS_World : public SS_Broadcaster, public SS_Listener, public SS_LayerList, public SS_CollisionManager
{
    private:
        SDL_Surface         *surface;

        SDL_Thread          *processThread, *renderThread;
        SDL_mutex           *worldMutex;
        bool                calibrating;
        Uint16              caliCount;
        worldEventProc      eventHandler;
        worldProc           preRenderProc;
        worldProc           postRenderProc;
        worldProc           postProcessor;

        float               view_w, view_h;
        float               zoom, zoom_w, zoom_h;
        SScolorf            clearColor;

        bool                processFlag, renderFlag, processQuit, renderQuit, worldQuit;

        SS_LayerItem        *pointerSprite;
        SS_Layer            *latchedLayer;

        Uint32              lastAutoTime;               // last time the auto fired
        Uint32              autoInterval;

    protected:
        SS_Game             *game;

    public:
        bool                fireAuto;                   // flag to fire auto move

        float               left, top;

        Uint16              slowCount;      // frame counting stuff
        Uint32              frameCount;
        Uint32              fps;

        Uint32              ticks;          // adjusted world time
        Uint32              timeAdjust;     // adjustment factor
        Uint32              stopTime;       // pause time started here

        Uint8               *keyState;
        float               mousex, mousey;
//      float               mousexrel, mouseyrel;
        Uint8               mouseButtons;
        bool                mouseClick, mouseDown;


        //-------------------------------------
        // SS_World Class Methods
        //
                            SS_World();
        virtual             ~SS_World();

        // Accessors
//      inline SS_CollisionManager* CollisionManager()  { return (SS_CollisionManager*)this; }

        inline float        Left() const            { return left; }
        inline float        Top() const             { return top; }
        inline float        Right() const           { return left + zoom_w; }
        inline float        Bottom() const          { return top + zoom_h; }
        inline float        Zoom() const            { return zoom; }
        inline float        ZoomWidth() const       { return zoom_w; }
        inline float        ZoomHeight() const      { return zoom_h; }
        inline float        ViewWidth() const       { return view_w; }
        inline float        ViewHeight() const      { return view_h; }
        inline SS_LayerItem* MousePointer() const   { return pointerSprite; }
        inline bool         ProcessFlag() const     { return processFlag; }
        inline Uint32       GetWorldTime() const    { return SDL_GetTicks() - timeAdjust; }
        inline bool         IsPaused() const        { return !processFlag; }

        // Setters
        inline void         SetLeftTop(float x, float y)        { left = x; top = y; }
        inline void         ScreenToGlobal(float *x, float *y)  { *x = *x * zoom + left; *y = *y * zoom + top; }
        inline void         SetZoom(float z)                    { zoom = z; zoom_w = view_w / z; zoom_h = view_h / z; }

        inline void         SetProcessFlag(bool f)              { processFlag = f; }
        void                SetPaused(bool p);
        inline void         Pause()                             { SetPaused(true); }
        inline void         Resume()                            { SetPaused(false); }
        inline void         TogglePaused()                      { SetPaused(processFlag); }

        void                Calibrate();
        void                SetSurface(SDL_Surface *s);

        // Layer general methods
        inline void         LatchLayer(SS_Layer *l)             { latchedLayer = l; }

        void                AddLayer(SS_Layer *layer);

        // Layer Creators
        SS_Layer*           NewLayer(Uint32 f=SS_NONE);

        SS_TileLayer*       NewTileLayer();
        SS_TileLayer*       NewTileLayer(SS_TileMap *map);
        SS_TileLayer*       NewTileLayer(SS_TileMap *map, Uint32 f);

        SS_TextLayer*       NewTextLayer(SS_SFont *sfont, Uint32 f);
        SS_TextLayer*       NewTextLayer(SS_SFont *sfont);
        SS_TextLayer*       NewTextLayer(char *filename, float desc, Uint32 f);
        SS_TextLayer*       NewTextLayer(char *filename, float desc);

        SS_GUI*             NewGUI();
        SS_GUI*             NewGUI(SS_SFont *font);

        // Layer Arrangement
        void                LayerToFront(SS_Layer *layer);
        void                LayerToBack(SS_Layer *layer);

        // Event Handling
        void                HandleEvents();
        virtual void        HandleEvent(SDL_Event *e);
        inline void         SetEventHandler(worldEventProc ep)  { eventHandler = ep; }
        void                SetPointerSprite(SS_LayerItem *s);
        SS_Sprite*          CreatePointerSprite(char *file);
        inline void         SetPreRenderProc(worldProc p)       { preRenderProc = p; }
        inline void         SetPostRenderProc(worldProc p)      { postRenderProc = p; }
        inline void         SetPostProcessor(worldProc p)       { postProcessor = p; }
        inline void         SetClearColor(SScolorb &q)          { SetClearColor(q.r, q.g, q.b, q.a); }
        inline void         SetClearColor(SScolorf &q)          { clearColor = q; }
        inline void         SetClearColor(GLubyte q[4])         { SetClearColor(q[0], q[1], q[2], q[3]); }
        inline void         SetClearColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a=255) { SetClearColor((float)r/255, (float)g/255, (float)b/255, (float)a/255); }
        inline void         SetClearColor(float r, float g, float b, float a=1.0f) { clearColor.r = r; clearColor.g = g; clearColor.b = b; clearColor.a = a; }

        // World Execution
        void                Start();
        Uint32              Run(SS_Game *g);
        void                Stop();
        inline void         Quit() { worldQuit = true; }

        void                GetInput();
        void                Process();
        void                Animate();
        void                Render();

        // Overridable Render and Process
        virtual void        PostProcess();
        virtual void        PreRender();
        virtual void        PostRender();

//      SS_ItemNode*        AddToColliders(SS_LayerItem *item);
//      void                RemoveFromColliders(SS_LayerItem *item);
//      void                RunCollisionTest();
//      SS_ItemList*        CollidersAtPoint(float x, float y);
//      SS_LayerItem*       FirstColliderOnLine(float x1, float y1, float x2, float y2);

        int                 ProcessThread();
        int                 RenderThread();

        static int          process_thread(void *world) { return ((SS_World*)world)->ProcessThread(); }

    private:
        void                Init();
};

#endif

