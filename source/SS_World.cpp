/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_World.cpp
 *
 *  $Id: SS_World.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_World.h"

#include "SS_Game.h"
#include "SS_Types.h"
#include "SS_Sprite.h"
#include "SS_Layer.h"
#include "SS_GUI.h"

// Useful OpenGL Globals
glState     gl_state;

extern bool SS_VSYNC;


//--------------------------------------------------------------
// SS_World
// All the layers in the whole world
//--------------------------------------------------------------

SS_World::SS_World()
{
    DEBUGF(1, "[%08X] SS_World() CONSTRUCTOR\n", this);

    Init();
}

SS_World::~SS_World()
{
    DEBUGF(1, "[%08X] ~SS_World() DESTRUCTOR\n", this);

    Stop();
    DisposeAll();
}

//
// Init
//
void SS_World::Init()
{
    DEBUGF(1, "[%08X] SS_World::Init()\n", this);

    eventHandler    = NULL;
    preRenderProc   = NULL;
    postRenderProc  = NULL;
    postProcessor   = NULL;

    worldMutex      = NULL;
    processThread   = NULL;
    renderThread    = NULL;

    calibrating     = false;
    caliCount       = 0;
    frameCount      = 0;
    slowCount       = 0;
    fps             = 0;
    timeAdjust      = 0;
    stopTime        = 0;

    processFlag     = false;
    renderFlag      = false;
    processQuit     = false;
    renderQuit      = false;
    worldQuit       = false;

    keyState        = NULL;
    mouseButtons    = 0x00;
    mouseClick      = false;
    mouseDown       = false;
    mousex          = 0;
    mousey          = 0;
    ticks           = 0;

    autoInterval    = 1000/100;
    lastAutoTime    = 0;
    fireAuto        = false;

    pointerSprite   = NULL;
    latchedLayer    = NULL;

    SetSurface(SS_Game::TheScreen());
    SetLeftTop(0, 0);
    SetZoom(1);
    SetClearColor(SS_BLACK_B);
}

//
// SetPointerSprite
//
void SS_World::SetPointerSprite(SS_LayerItem *s)
{
    DEBUGF(1, "[%08X] SS_World::SetPointerSprite(%08X)\n", this, s);

    if (pointerSprite) {
        delete pointerSprite->Layer();
        pointerSprite = NULL;
    }

    if ((pointerSprite = s)) {
        SS_Layer *layer = new SS_Layer(SS_NOZOOM|SS_NOSCROLL);
        layer->AddItem(s);
        AddLayer(layer);
        s->Move(mousex, mousey);
        s->Hide();
    }
}

//
// CreatePointerSprite
//
SS_Sprite* SS_World::CreatePointerSprite(char *file)
{
    DEBUGF(1, "SS_World::CreatePointerSprite(file)\n");

    SS_Sprite   *sprite = new SS_Sprite();
    SS_Frame    *frame = new SS_Frame(file);
    sprite->AddFrame(frame);
    SetPointerSprite(sprite);
    return sprite;
}

//
// Run
// The entire world runs from here
//
Uint32 SS_World::Run(SS_Game *g)
{
    DEBUGF(1, "[%08X] SS_World::Run(%08X)\n", this, g);

    Uint32  interval, old, tick;

    game = g;
    if (g->IsQuitting())
        return 0;

    if (SS_VSYNC)
        Calibrate();

    Start();

    while ( !worldQuit && !game->IsQuitting())              // while the world has not been quit
    {
#if !SS_THREADS
        ticks = GetWorldTime();
#endif

        GetInput();                         // get a snapshot of all input states
        HandleEvents();                     // handle events

#if !SS_THREADS

        if (processFlag)
        {
            static Uint32 collTick = 0;
            if (GetWorldTime() - collTick > 5)
            {
                collTick = GetWorldTime();
                RunCollisionTest();
            }

            Process();
            Animate();
        }
#endif

        if (renderFlag)                         // if rendering is enabled
        {
            Render();                           // do so

            frameCount++;                       // calculate FPS every 2 seconds
            tick = GetWorldTime();
            interval = tick - old;
            if (interval >= 2000)
            {
                fps = (int)(1000.0 * frameCount / interval + 0.5);
                old = tick;
                frameCount = 0;
            }
        }
    }

    Stop();

    return fps;
}

//
// Start
// Start the processing thread and enable rendering for the world
//
void SS_World::Start()
{
    DEBUGF(1, "[%08X] SS_World::Go()\n", this);

    worldQuit = false;
    processQuit = false;
    processFlag = false;

#if SS_THREADS
    worldMutex = SDL_CreateMutex();
    processThread = SDL_CreateThread(SS_World::process_thread, this);
#endif

    renderQuit = false;
    renderFlag = false;
//  renderThread = SDL_CreateThread(render_thread, this);

    Resume();
    renderFlag = true;
}

//
// Stop
// Stop all processing and rendering
//
void SS_World::Stop()
{
    DEBUGF(1, "[%08X] SS_World::Stop()\n", this);

    int dummy;

    Pause();
    renderFlag = false;

    if (processThread) {
        processQuit = true;
        SDL_WaitThread(processThread, &dummy);
        processThread = NULL;
    }

/*
    if (renderThread) {
        renderQuit = true;
        SDL_WaitThread(renderThread, &dummy);
        renderThread = NULL;
    }
*/

    if (worldMutex) {
        SDL_DestroyMutex(worldMutex);
        worldMutex = NULL;
    }
}

//
// SetPaused
// Pause or resume processing, keeping track of paused time
//
void SS_World::SetPaused(bool p)
{
    processFlag = !p;

    if (p == true)
        stopTime = SDL_GetTicks();
    else if (stopTime != 0)
        timeAdjust += (SDL_GetTicks() - stopTime);
}

//
// Calibrate
// Start testing the frame rate to see if VBL Sync is okay
//
void SS_World::Calibrate()
{
    DEBUGF(1, "[%08X] SS_World::Calibrate()\n", this);

    frameCount = 0;

    SS_Game::SyncVblank(1);

    calibrating = true;
    caliCount = 120;
    slowCount = 0;
}

//
// SetSurface(surface)
// Tell the world which SDL_Surface to use for drawing
//
void SS_World::SetSurface(SDL_Surface *s)
{
    DEBUGF(1, "[%08X] SS_World::SetSurface(%08X)\n", this, s);

    int w, h;

    if ((surface = s))
    {
        w = s->w;
        h = s->h;
    }
    else
        w = h = 0;

    view_w = w;
    view_h = h;
}

//
// GetInput
// Update the keyboard and mouse states
//
void SS_World::GetInput()
{
    int x, y;

    keyState = SDL_GetKeyboardState(NULL);

    mouseButtons = SDL_GetMouseState(&x, &y);
}

//
// Process
// Tell all the layers in the world to Process
//

void SS_World::Process()
{
    if ((fireAuto = (ticks - lastAutoTime >= autoInterval)))
        lastAutoTime = ticks;

    SS_Layer            *layer;
    SS_LayerIterator    itr = GetIterator();

    while ((layer = itr.NextItem()))
        if (layer->enabled && !layer->paused)
            layer->Process();

    // Delete marked layers
    itr.Start();
    while ((layer = itr.NextItem()))
        if (layer->removeFlag)
            delete layer;

    PostProcess();
}

//
// PostProcess
// Process something after all the layers
//
void SS_World::PostProcess()
{
    if (postProcessor)
        postProcessor(this);
}

//
// Animate
// Tell all the layers in the world to Animate
//
void SS_World::Animate()
{
    SS_Layer            *layer;
    SS_LayerIterator    itr = GetIterator();

    while ((layer = itr.NextItem()))
        if (layer->enabled && !layer->paused)
            layer->Animate();
}

//
// Render
// Render the world and display it
//
void SS_World::Render()
{
    Uint32       tick = SDL_GetTicks();

    #if SS_THREADS
    SDL_LockMutex(worldMutex);  // No processing in this portion
    #endif

    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);       // Clear the display buffer

    PreRender();

    SS_Layer            *layer;
    SS_LayerIterator    itr = GetIterator();

    while ((layer = itr.NextItem()))
        if (layer->enabled)
            layer->Render();

    PostRender();

    //
    // Swap buffers and allow processing to continue.
    // Theoretically, if you unlock the processing thread
    // before swapping buffers the swap will occur soon enough
    // to maintain smooth motion. However the threading model
    // of Windows 98 seems to spool up time.
    //
    // So for the Windows build we swap the buffers first, then
    // unlock the processor. On modern machines this works fine.
    //
    #ifdef WIN32
    SDL_GL_SwapBuffers();
    #endif

    #if SS_THREADS
    SDL_UnlockMutex(worldMutex);
    #endif

    #ifndef WIN32
    SDL_GL_SwapWindow(window);
    #endif

    //
    // Calibration test
    //
    if (calibrating)
    {
        if (SDL_GetTicks() - tick > (1000 / 50))
            slowCount++;

        if (!--caliCount)
        {
            calibrating = false;

            if (slowCount >= 60)
                SS_Game::SyncVblank(0);
        }
    }
}

//
// PreRender
// Render something before everything else
//
void SS_World::PreRender()
{
    if (preRenderProc != NULL)
        preRenderProc(this);
}

//
// PostRender
// Render something after everything else
//
void SS_World::PostRender()
{
    if (postRenderProc != NULL)
        postRenderProc(this);
}

//
// ProcessEvents
// The thread for processing a world
//
void SS_World::HandleEvents()
{
    SDL_Event   event;
    float       x, y;
    int         k;

    while (SDL_PollEvent(&event))
    {
        // preprocess some events
        switch (event.type)
        {
            case SDL_MOUSEMOTION:
//              mousexrel = h = event.motion.xrel;
//              mouseyrel = v = event.motion.yrel;

                mousex = x = event.motion.x;
                mousey = y = event.motion.y;

                if (pointerSprite)
                {
                    pointerSprite->Move(x, y);
/*
                    if (h == 0.0 && v == 0.0)
                        pointerSprite->Hide();
                    else
*/
                    if ( !pointerSprite->IsVisible() )
                        pointerSprite->Show();
                }

                break;

            case SDL_MOUSEBUTTONDOWN:
                mouseDown = true;
                mouseClick = true;
                break;

            case SDL_MOUSEBUTTONUP:
                mouseDown = false;
                break;

            case SDL_KEYDOWN:
                k = event.key.keysym.sym;
                if (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT | KMOD_CAPS) && (k >= 'a' && k <= 'z'))
                {
                    k -= ('a' - 'A');
                }
                else if (event.key.keysym.mod & KMOD_SHIFT)
                {
                    switch(k)
                    {
                        case SDLK_BACKQUOTE:
                            k = SDLK_UNKNOWN;
                            break;

                        case SDLK_1:
                            k = SDLK_EXCLAIM;
                            break;

                        case SDLK_2:
                            k = SDLK_AT;
                            break;

                        case SDLK_3:
                            k = SDLK_HASH;
                            break;

                        case SDLK_4:
                            k = SDLK_DOLLAR;
                            break;

                        case SDLK_5:
                            k = SDLK_UNKNOWN;
                            break;

                        case SDLK_6:
                            k = SDLK_CARET;
                            break;

                        case SDLK_7:
                            k = SDLK_AMPERSAND;
                            break;

                        case SDLK_8:
                            k = SDLK_ASTERISK;
                            break;

                        case SDLK_9:
                            k = SDLK_LEFTPAREN;
                            break;

                        case SDLK_0:
                            k = SDLK_RIGHTPAREN;
                            break;

                        case SDLK_MINUS:
                            k = SDLK_UNDERSCORE;
                            break;

                        case SDLK_EQUALS:
                            k = SDLK_PLUS;
                            break;

                        case SDLK_SEMICOLON:
                            k = SDLK_COLON;
                            break;

                        case SDLK_QUOTE:
                            k = SDLK_QUOTEDBL;
                            break;

                        case SDLK_COMMA:
                            k = SDLK_LESS;
                            break;

                        case SDLK_PERIOD:
                            k = SDLK_GREATER;
                            break;

                        case SDLK_SLASH:
                            k = SDLK_QUESTION;
                            break;
                    }
                }

                if (k)
                    event.key.keysym.sym = (SDL_Keycode)k;
                else
                    return;

                break;

            case SDL_QUIT:
                Quit();
                return;
        }

        bool        did = false;

        //
        // Ask all the layers to handle the event
        //
        SS_Layer            *layer;
        SS_LayerIterator    itr = GetIterator();

        //
        // TODO: Implement an event registry scheme so
        // only eligible items get asked to handle events
        //
        itr.End();
        while ((layer = itr.PreviousItem()))
            if ((layer->enabled && !layer->paused) && (latchedLayer == NULL || latchedLayer == layer) && (did = layer->HandleEvent(&event)))
            {
                break;
            }

        //
        // Activate the layer that handled the last event, even if none did.
        //
        // This currently looks for layers of type GUI and "Activates" them.
        // Presumably, a layer that handles an event should also be able to
        // decide whether it will activate upon completion. This behavior might
        // be movable into the SS_GUI class.
        //
        if (layer == NULL)
            SS_GUI::ActivateNone();
        else if (layer->Type() == SS_LAYER_GUI)
            ((SS_GUI*)layer)->Activate();

        // If nothing else handled the event then call the world event handler
        if (!did)
            HandleEvent(&event);

/*
        // This is the alternative idea mentioned below...
        if (!did)
            if (!HandleEvent(&event))
                gameObject->HandleEvent(&event);
*/

        mouseClick = false;
    }
}

//
// HandleEvent
//
// If the world subclass doesn't override this method
// then this default method is called. It calls the
// event handler proc which might have been set.
// After that we're done.
// An alternative approach would be to have a fallback
// handler in SS_Game (and so in YourGame also) as shown
// above.
//
void SS_World::HandleEvent(SDL_Event *event)
{
    if (eventHandler)
        (*eventHandler)(this, event);
}

//
// ProcessThread
// The thread for processing a world
//
int SS_World::ProcessThread()
{
    while (!processQuit)
    {
        ticks = GetWorldTime();

        if (processFlag)
        {
            static Uint32 collTick = 0;
            if (GetWorldTime() - collTick > 5)
            {
                collTick = GetWorldTime();
                RunCollisionTest();
            }

            SDL_LockMutex(worldMutex);      // increment the mutex, and if >1 then wait
            Process();
            Animate();
            SDL_UnlockMutex(worldMutex);    // decrement the mutex
            SDL_Delay(5);
        }
    }

    return 0;
}

//
// RenderThread
// The thread for rendering a world
//
int SS_World::RenderThread()
{
    while (!renderQuit)
    {
        if (renderFlag)
        {
            Render();
            frameCount++;
        }
        SDL_Delay(1);
    }

    return 0;
}

//
// A thread for rendering
//
int render_thread(void *world)
{
    return ((SS_World*)world)->RenderThread();
}
