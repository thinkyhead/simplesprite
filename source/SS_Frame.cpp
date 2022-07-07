/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Frame.cpp
 *
 *  $Id: SS_Frame.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Frame.h"

#include "SS_Utilities.h"

#include <stdlib.h>


//--------------------------------------------------------------
// SS_Frame
// A single image for use in sprites
//--------------------------------------------------------------

SS_Frame::SS_Frame(char* filename, frameFlags f)
{
    Init(f);
    LoadImage(filename);
}

SS_Frame::SS_Frame(SDL_Surface *surf, frameFlags f)
{
    Init(f);
    LoadSurface(surf);
}

SS_Frame::~SS_Frame()
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::~SS_Frame()\n", this);
    #endif

    DisposeSurface();
    DisposeMask();
    DisposeTexture();

    if (gl_list) {
        glDeleteLists(gl_list, 1);
        gl_list = 0;
    }
}

//
// Init(flags)
//
void SS_Frame::Init(frameFlags f)
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::Init()\n", this);
    #endif

    flags           = f;
    surface         = NULL;
    mask            = NULL;
    gl_texture      = 0;
    gl_list         = 0;
    texw            = 0;
    texh            = 0;
    width           = 0;
    height          = 0;

    SetTint(255, 255, 255, 255);
}

//
// SetHandle(x,y)
//
void SS_Frame::SetHandle(float x, float y)
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::SetHandle(%d, %d)\n", this, x, y);
    #endif

    xhandle = x;
    yhandle = y;
    InitDisplayList();
}

//
// LoadImage
// Load an image from a file into the frame
//
void SS_Frame::LoadImage(const char *filename)
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::LoadImage(\"%s\")\n", this, filename);
    #endif

    SDL_Surface **save;

    DisposeSurface();
    DisposeMask();
    DisposeTexture();

    // frames can choose to keep the original surface around
    // for use in collisions or whatever.
    if (flags & (SS_KEEP_SURFACE | SS_COLLISION_MASK))
        save = &surface;
    else
        save = NULL;

    ::GetTextureFromImage(filename, &width, &height, &gl_texture, &texw, &texh, save);

    if (flags & SS_COLLISION_MASK) {
        MakeCollisionMask();
        if (!(flags & SS_KEEP_SURFACE))
            DisposeSurface();
    }

    CenterHandle();
}

//
// LoadSurface
// Load an image from a surface into the frame
//
void SS_Frame::LoadSurface(SDL_Surface *surf, const SDL_Rect *section)
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::LoadSurface(s)\n", this);
    #endif

    if (surface != surf)
    {
        DisposeSurface();
        DisposeMask();
    }

    DisposeTexture();

    if (section != NULL)
    {
    }
    else
    {
        if (!::MakeTextureFromSurface(surf, NULL, &gl_texture, &texw, &texh))
            throw "OpenGL texture could not be created.";

        width = surf->w;
        height = surf->h;

        if (surface == NULL && (flags & (SS_COLLISION_MASK|SS_KEEP_SURFACE)))
        {
            surface = surf;

            if (flags & SS_COLLISION_MASK)
            {
                MakeCollisionMask();
                if (!(flags & SS_KEEP_SURFACE))
                    surface = NULL;
            }
        }
    }

    CenterHandle();
}

//
// DisposeSurface
// Dispose the saved surface, if any
//
void SS_Frame::DisposeSurface()
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::DisposeSurface(s)\n", this);
    #endif

    if (surface) {
        free(surface);
        surface = NULL;
    }
}

//
// DisposeMask
// Dispose the collision mask, if any
//
void SS_Frame::DisposeMask()
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::DisposeMask(s)\n", this);
    #endif

    if (mask) {
        free(mask);
        mask = NULL;
    }
}

//
// DisposeTexture
// Dispose the GL texture, if any
//
void SS_Frame::DisposeTexture()
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::DisposeTexture(s)\n", this);
    #endif

    if (gl_texture) {
        glDeleteTextures(1, &gl_texture);
        gl_texture = 0;
    }
}

//
// DisposeDisplayList
// Dispose the GL display list, if any
//
void SS_Frame::DisposeDisplayList()
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::DisposeDisplayList(s)\n", this);
    #endif

    if (gl_list) {
        glDeleteLists(1, gl_list);
        gl_list = 0;
    }
}

//
// MakeCollisionMask
// Create a one-bit-per-pixel collision mask
//
void SS_Frame::MakeCollisionMask(Uint8 threshold)
{
    #if SS_DEBUG
    printf("[%08X] SS_Frame::MakeCollisionMask(%02X)\n", this, threshold);
    #endif

    if (!surface)
        throw "MakeCollisionMask requires a surface.";

    Uint16  w = surface->w, h = surface->h;
    Uint16  bw = (w + 7) / 8, b;
    Uint8   m = 0, *a;

    mask = (Uint8*)calloc(h, bw);
    maskBytesWide = bw;

    if (!mask)
        throw "Can't get memory for a collision mask.";

    a = mask;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            b = x % 8;
            if (b == 0) m = 0;

            if (SS_ALPHA(::GetPixel(surface, x, y)) > threshold)
            {
                m |= (1 << b);
#if SS_DEBUG
                printf("*");
#endif
            }

#if SS_DEBUG
            else
                printf(".");
#endif

            if (b == 7 || x == w-1)
                *a++ = m;
        }

#if SS_DEBUG
        printf("\n");
#endif
    }
}


//
// Render
// Render the frame at the given screen coordinates
// with the given rotation, scaling, and color tint
//
void SS_Frame::Render(float x, float y, float rot, float xscale, float yscale, const SScolorb &tintAdd)
{
    SScolorb aTint;
    MultiplyColorQuads(tint, tintAdd, aTint);

    gl_do_texture(1);
    gl_do_blend(1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(x, y, 0.0f);
    glRotatef(rot, 0.0f, 0.0f, 1.0f);
    glScalef(xscale, yscale, 1.0f);
    glColor4ubv((GLubyte*)&aTint);

    glCallList(gl_list);

    glPopMatrix();
}


//
// SendGeometry
//
void SS_Frame::SendGeometry()
{
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTranslatef(-xhandle, -yhandle, 0.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(texw, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(texw, texh);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, texh);
    glEnd();
}


//
// InitDisplayList
//
void SS_Frame::InitDisplayList()
{
    if (gl_list == 0)
        gl_list = glGenLists(1);

    glNewList(gl_list, GL_COMPILE);
    SendGeometry();
    glEndList();
}

