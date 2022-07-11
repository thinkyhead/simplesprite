/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Types.h
 *
 *  $Id: SS_Types.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_TYPES_H__
#define __SS_TYPES_H__

#include "SS_Config.h"

#define DEBUGF(N,V...) if (SS_DEBUG >= N) printf(V)

//
// Forward Class Declarations
//

class SS_AI;
class SS_Broadcaster;
class SS_Button;
class SS_Checkbox;
class SS_CustomGadget;
class SS_DataContext;
class SS_DataToken;
class SS_Dragger;
class SS_FlatFile;
class SS_Folder;
class SS_FolderIterator;
class SS_Frame;
class SS_Gadget;
class SS_Game;
class SS_GUI;
class SS_ItemGroup;
class SS_Layer;
class SS_LayerItem;
class SS_Listener;
class SS_RadioButton;
class SS_Scrollbar;
class SS_SFont;
class SS_Slider;
class SS_Sound;
class SS_Sprite;
class SS_String;
class SS_TextInput;
class SS_TextLayer;
class SS_TileLayer;
class SS_TileMap;
class SS_World;


// Texture Flags
enum frameFlags {
    SS_FRAME_NONE       = 0L,
    SS_COLLISION_MASK   = (1L << 0),
    SS_KEEP_SURFACE     = (1L << 1),
    SS_NO_BLEND_MAX     = (1L << 2),
    SS_NO_BLEND_MIN     = (1L << 3),
    SS_BLEED_S          = (1L << 4),
    SS_BLEED_T          = (1L << 5),
    SS_NO_ANTIALIAS     = (SS_NO_BLEND_MAX|SS_NO_BLEND_MIN),
    SS_BLEED            = (SS_BLEED_S|SS_BLEED_T)
    };

//
// Sprite and Layer flag bits
//
enum {
    SS_NONE     = 0L,
    SS_AUTOMOVE = (1L << 0),
    SS_NOSCROLL = (1L << 1),
    SS_RADAR    = (1L << 2),
    SS_NOZOOM   = (1L << 3),
    SS_ABSROT   = (1L << 4)
};

//
// String Flags
//
enum stringAlign {
    SA_LEFT         = (1L << 0),
    SA_RIGHT        = (1L << 1),
    SA_CENTERH      = SA_LEFT|SA_RIGHT,

    SA_ABOVE        = (1L << 2),
    SA_BELOW        = (1L << 3),
    SA_BASELINE     = (1L << 4),
    SA_CENTERV      = SA_ABOVE|SA_BELOW,

    SA_CENTER       = SA_CENTERH|SA_CENTERV,

    SA_HMASK        = SA_LEFT|SA_RIGHT,
    SA_VMASK        = SA_ABOVE|SA_BELOW|SA_BASELINE
};

//
// Required Headers for this file
//
#include <OpenGL/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>

#ifndef CALLBACK
#define CALLBACK
#endif

//
// Useful macros
//
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define YES true
#define NO  false

#define NUM_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))
#define SWAP(a,b,t) do {t=a;a=b;b=t;} while (0)
#ifndef MIN
    #define MIN(x, y)   (((x) < (y)) ? x : y)
    #define MAX(x, y)   (((x) > (y)) ? x : y)
#endif
#define ABS(x)      ((x) < 0 ? -(x) : (x))
#define RAD(d)      ((d) * M_PI / 180)
#define DEG(r)      ((r) * 180 / M_PI)

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define SS_ALPHA(c) ((c) & 0xFF)
    #define SS_COLOR(c) ((c) >> 8)
    #define SS_RGB(c) ((c) >> 8)
    #define SS_RGBA(c) (c)
#else
    #define SS_ALPHA(c) ((c) >> 24)
    #define SS_COLOR(c) ((c) & 0xFFFFFF)
    #define SS_RGB(c) ( (((c) & 0x00FF0000) >> 16) | ((c) & 0x0000FF00) | (((c) & 0x000000FF) << 16) )
    #define SS_RGBA(c) ( (((c) & 0xFF000000) >> 24) | (((c) & 0x00FF0000) >> 8) | (((c) & 0x00FF0000) << 8) | (((c) & 0x000000FF) << 24) )
#endif

#ifdef WIN32
    #define random()        rand()
    #define srandom(t)      srand(t)
    #define RANDINT(x, y)   (rand() % (long)((y)-(x)+1) + (x))
    #define RANDSGN         ((rand() & 2) - 1)
#else
    #define RANDINT(x, y)   (random() % (long)((y)-(x)+1) + (x))
    #define RANDSGN         ((random() & 2) - 1)
#endif

#define RANDFLOAT(x, y) (float)RANDINT(x,y)

#define SS_ROTINDEX(x)  ((Uint16)floorf((x) * 65536.0f / 360.0f))
#define SS_ROTANGLE(x)  ((float)x * 360.0f / 65536.0f)
#define BASEANGLE(x)    SS_ROTANGLE(SS_ROTINDEX(x))

//
// OpenGL ubyte-style color quad
//
typedef struct {
    GLubyte r, g, b, a;
} SScolorb;

//
// GUI element Color set for interactive tinting
//
typedef struct {
    GLfloat     borderWeight;               // Weight of the border line
    SScolorb    borderColor;                // Color for the border, when enabled
    SScolorb    fillColor;                  // Color to fill or to tint a sprite
    SScolorb    labelColor;                 // Tint to apply to the text, if any
} colorSet, *colorSetPtr;

//
// OpenGL float-style color quad
//
typedef struct {
    GLfloat r, g, b, a;
} SScolorf;

//
// A simple point
//
typedef struct {
    float   x, y, r;
    Uint16  i;
} SS_Point;

//
// Our preferred rectangle struct
//
typedef struct {
    float   x, y, w, h;
} SS_Rect;

//
// OpenGL current state and inline methods
//
typedef struct {
    bool    do_blend;
    bool    do_texture;
    bool    antialias;
    GLuint  texture_id;
    GLfloat line_width;
    GLenum  poly_mode;
} glState;

extern glState gl_state;

static __inline__ void gl_do_blend(bool on)
{
    if(gl_state.do_blend == on)
        return;

    if(on)  glEnable(GL_BLEND);
    else    glDisable(GL_BLEND);

    gl_state.do_blend = on;
}

static __inline__ void gl_do_texture(bool on)
{
    if(gl_state.do_texture == on)
        return;

    if(on)  glEnable(GL_TEXTURE_2D);
    else    glDisable(GL_TEXTURE_2D);

    gl_state.do_texture = on;
}

static __inline__ void gl_bind_texture(GLuint tx)
{
    if(gl_state.texture_id == tx)
        return;

    glBindTexture(GL_TEXTURE_2D, tx);
    gl_state.texture_id = tx;
}

static __inline__ void gl_line_width(GLfloat lw)
{
    glLineWidth(lw);
}

static __inline__ void gl_antialias(bool on)
{
    if(gl_state.antialias == on)
        return;

    if(on)  { glEnable(GL_LINE_SMOOTH); gl_state.line_width = 1; }
    else    glDisable(GL_LINE_SMOOTH);

    gl_state.antialias = on;
}

static __inline__ void gl_poly_mode(GLenum mode)
{
    if(gl_state.poly_mode == mode)
        return;

    glPolygonMode(GL_FRONT_AND_BACK, mode);

    gl_state.poly_mode = mode;
}

//
// SS Linked List types
//
#include "SS_Templates.h"

typedef TListNode<char*>        SS_CharNode;                // char
typedef TLinkedList<char*>      SS_CharList;
typedef TIterator<char*>        SS_CharIterator;

typedef TListNode<SS_World*>    SS_WorldNode;               // SS_World
typedef TObjectList<SS_World*>  SS_WorldList;
typedef TIterator<SS_World*>    SS_WorldIterator;

typedef TListNode<SS_Layer*>    SS_LayerNode;               // SS_Layer
typedef TObjectList<SS_Layer*>  SS_LayerList;
typedef TIterator<SS_Layer*>    SS_LayerIterator;

typedef TListNode<SS_Sprite*>   SS_SpriteNode;              // SS_Sprite
typedef TObjectList<SS_Sprite*> SS_SpriteList;
typedef TIterator<SS_Sprite*>   SS_SpriteIterator;

typedef TListNode<SS_Gadget*>   SS_GadgetNode;              // SS_Gadget
typedef TObjectList<SS_Gadget*> SS_GadgetList;
typedef TIterator<SS_Gadget*>   SS_GadgetIterator;

//
// Event codes
//
enum ssEventCode {
    SS_EVENT_IDLE,
    SS_EVENT_CLICK,
    SS_EVENT_DRAG,
    SS_EVENT_UNCLICK,
    SS_EVENT_ENTER,
    SS_EVENT_MOVE,
    SS_EVENT_EXIT,
    SS_EVENT_KEYDOWN,
    SS_EVENT_KEYUP,
    SS_EVENT_GAIN_FOCUS,
    SS_EVENT_LOSE_FOCUS,
    SS_EVENT_CHANGE
};

//
// Our own event structure
//
typedef struct {
    ssEventCode type;
    float       x, y;
//  float       xrel, yrel;
    float       xgui, ygui;
    float       xglob, yglob;
    SDLKey      key;
    SDLMod      mod;
} SS_Event;

#endif
