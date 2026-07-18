/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Game.cpp
 *
 *  $Id: SS_Game.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Game.h"
#include "SS_Files.h"

#include "SS_Types.h"
#include "SS_Layer.h"
#include "SS_LayerItem.h"
#include "SS_World.h"
#include "SS_Sound.h"

#include "SS_sdl3.h"        // SDL 1.2 -> 3.x bridge

#include <SDL_opengl.h>

#include <string.h>


//
// Get headers to allow VSync
//
#ifdef WIN32

typedef void (APIENTRY *SWAP_PROC)(int interval);

#elif defined(__APPLE__)

#include <OpenGL/CGLCurrent.h>
#include <OpenGL/OpenGL.h>

#endif

#undef SS_VIDEO_W
#undef SS_VIDEO_H
#undef SS_VSYNC
#undef SS_FULLSCREEN

int     SS_VIDEO_W = 1024;
int     SS_VIDEO_H = 768;
bool    SS_VSYNC = 0;
bool    SS_FULLSCREEN = 0;

//--------------------------------------------------------------
// SS_Game
//--------------------------------------------------------------

// Static Initializers
SDL_Surface *SS_Game::ss_screen  = NULL;
SDL_Window  *SS_Game::ss_window  = NULL;
SDL_GLContext SS_Game::ss_glcontext = NULL;
float       SS_Game::SS_cos[65536];
float       SS_Game::SS_sin[65536];
SS_SFont    *SS_Game::tinyFont = NULL, *SS_Game::smallFont = NULL, *SS_Game::mediumFont = NULL, *SS_Game::largeFont = NULL;

SS_Game::SS_Game()
{
    LoadConfig();
    Init();
}


SS_Game::~SS_Game()
{
    while (worldCount)
        PopWorld();
}

//
// Load Game Config
//
void SS_Game::LoadConfig()
{
    SS_Folder::cdAppFolder();

    SS_FlatFile   dataFile("ssgame.cfg");

    dataFile.EnterContext("SS");

    if (!dataFile.Exists()) {
        dataFile.SetToken("Width", SS_VIDEO_W);
        dataFile.SetToken("Height", SS_VIDEO_H);
        dataFile.SetToken("Fullscreen", SS_FULLSCREEN);
        dataFile.SetToken("Vsync", SS_VSYNC);
        dataFile.Export();
    }

    SS_VIDEO_W = dataFile.GetInteger("Width");
    SS_VIDEO_H = dataFile.GetInteger("Height");
    SS_FULLSCREEN = dataFile.GetBoolean("Fullscreen");
    SS_VSYNC = dataFile.GetBoolean("Vsync");
}

//
// Init
//
void SS_Game::Init()
{
    world           = NULL;
    bQuit           = false;

    worldCount  = 0;
    for (int i=SS_MAX_WORLDS;i--;)
        worlds[i] = NULL;

    InitTrigonometry();             // Prepare sine and cosine arrays
    InitScreen();                   // Prepare the screen / window
    SS_Sound::InitAudioMixer(64);   // Start up the audio system

    // Send all printed output to a file
    SS_Folder::cdAppFolder();
    FILE *stream = freopen(SS_Folder::FullPath("ssgame.out"), "w", stdout);
    if (stream == NULL) exit(-1);
    fprintf(stdout, "Testing 123...\n");

    SS_Folder::SetWorkingDir("");
}

//
// InitScreen
// Start up the video system with OpenGL support
//
void SS_Game::InitScreen()
{
    //
    // Get a screen or a window
    //
    if ( !SDL_Init( (SDL_INIT_VIDEO|(SS_JOYSTICK_ENABLE ? SDL_INIT_JOYSTICK : 0x00)|(SS_AUDIO_ENABLE ? SDL_INIT_AUDIO : 0x00)) ) )
        throw "Couldn't initialize SDL: %s\n";

    int bpp = 32;   // SDL 1.2 picked the desktop depth; 32 is the safe GL default

    //
    // SDL 2.x: create a window + OpenGL context instead of SDL_SetVideoMode
    //
    Uint32 windowFlags = SDL_WINDOW_OPENGL | (SS_FULLSCREEN ? SDL_WINDOW_FULLSCREEN : 0x00);
    ss_window = SDL_CreateWindow("SimpleSprite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SS_VIDEO_W, SS_VIDEO_H, windowFlags);
    if (!ss_window)
        throw "Can't create Window: %s\n";

    ss_glcontext = SDL_GL_CreateContext(ss_window);
    if (!ss_glcontext)
        throw "Can't create GL context: %s\n";

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_SetWindowGrab(ss_window, SDL_TRUE);     // Keep mouse pointer in window
    SDL_ShowCursor(SDL_DISABLE);                // Hide the system pointer

    //
    // Turn off 3D features
    //
    glDisable(GL_DEPTH_TEST);                       // Depth testing
    glDisable(GL_CULL_FACE);                        // Face Culling

    //
    // The Viewport clears to black
    //
    glViewport(0, 0, SS_VIDEO_W, SS_VIDEO_H);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);

    //
    // Flat 2D rendering: NO OpenGL lighting. Enabling GL_LIGHTING (with no
    // normals/light model set up) drove the fixed-function lighting equation
    // to zero and erased translucent textured sprites (reticle, title text).
    // Tinting is done via the texture environment (GL_MODULATE) combined with
    // glColor, so every sprite/frame draws with its own tint + alpha.
    //
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    //
    // Would like to see smoother edges on rounded rect, for example
    //
//  glEnable(GL_POLYGON_SMOOTH);

    //
    // Init the model matrix with Identity and blending as Alpha
    //
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//  gl_state.do_blend   = glIsEnabled(GL_BLEND);
//  gl_state.do_texture = glIsEnabled(GL_TEXTURE_2D);
//  gl_state.antialias  = glIsEnabled(GL_LINE_SMOOTH);
//  gl_state.texture_id = 0;
//  glGetIntegerv(GL_LINE_WIDTH, &gl_state.line_width);
//  GLint modes[2]; glGetIntegerv(GL_POLYGON_MODE, modes);
//  gl_state.poly_mode = modes[1];

}

//
// InitTrigonometry
//
void SS_Game::InitTrigonometry()
{
    //
    // ANGULAR VECTORS
    //
    double r;
    for (unsigned i=65536; i--;) {
        r = i * M_PI / 32768.0;
        SS_cos[i] = cos(r);
        SS_sin[i] = sin(r);
    }
}

//
// SyncVblank(interval)
//
// Tell OpenGL to synchronize to vertical blanking
//
void SS_Game::SyncVblank(long sync)
{
#ifdef WIN32

    SWAP_PROC wglSwapIntervalEXT = (SWAP_PROC) wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT != NULL) wglSwapIntervalEXT(sync);

#elif defined(__APPLE__)

    CGLContextObj   ctx = CGLGetCurrentContext();
    GLint           interval = sync;

    // tell OpenGL to synchronize with screen refresh
    CGLSetParameter(ctx, kCGLCPSwapInterval, &interval);

#endif
}

//
// PushWorld
//
void SS_Game::PushWorld(SS_World *w)
{
    if (worldCount >= SS_MAX_WORLDS)
        throw "World stack is full.";

    worlds[worldCount++] = w;
    world = w;
}


//
// PopWorld
//
void SS_Game::PopWorld()
{
    if (worldCount <= 0)
        throw "World stack is empty.";

    delete world;

    if (--worldCount)
        world = worlds[worldCount - 1];
    else
        world = NULL;
}

/*

//
// HandleEvent
// placeholder for the fallback event handler
//
void SS_Game::HandleEvent(SDL_Event *event)
{
}

*/
