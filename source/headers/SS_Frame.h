/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  $Id: SS_Frame.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_FRAME_H__
#define __SS_FRAME_H__

#include "SS_Types.h"
#include "SS_RefCounter.h"

//--------------------------------------------------------------
// SS_Frame
//

class SS_Frame : public SS_RefCounter
{
    friend class SS_Sprite;

    private:
        SScolorb        tint;               // a color to tint the frame when drawn
        GLuint          gl_list;            // OpenGL display list
        Uint16          texw, texh;         // OpenGL texture size (powers of 2)
        SDL_Surface     *surface;           // SDL surface, if preserved

        frameFlags      flags;              // frame-specific flags

    protected:
        Uint8           *mask;              // collision mask, if created
        Uint16          maskBytesWide;      // useful value

    public:
        GLuint          gl_texture;         // OpenGL texture
        float           xhandle, yhandle;   // handle for relative positioning
        float           width, height;      // size of image area

                        SS_Frame(char *filename, frameFlags f=SS_FRAME_NONE);
                        SS_Frame(SDL_Surface *s, const SDL_Rect *section, frameFlags f=SS_FRAME_NONE);
                        SS_Frame(SDL_Surface *s, frameFlags f=SS_FRAME_NONE);
                        ~SS_Frame();

        inline float    Height() { return height; }
        inline float    Width() { return width; }

        void            LoadImage(const char *filename);
        void            LoadSurface(SDL_Surface *s, const SDL_Rect *section=NULL);
        inline void     SendGeometry();
        void            InitDisplayList();
        inline void     MakeCollisionMask() { MakeCollisionMask(0x00); }
        void            MakeCollisionMask(Uint8 t);
        void            DisposeMask();
        void            DisposeSurface();
        void            DisposeTexture();
        void            DisposeDisplayList();
        inline void     SetTint(SScolorb &t) { tint = t; }
        inline void     SetTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { tint.r = r;  tint.g = g;  tint.b = b;  tint.a = a; }
        inline void     SetTint(GLubyte r, GLubyte g, GLubyte b) { tint.r = r;  tint.g = g;  tint.b = b; }
        inline void     SetAlpha(GLubyte a) { tint.a = a; }
        void            SetHandle(float x, float y);
        inline void     GetHandle(float *x, float *y) { *x = xhandle; *y = yhandle; }
        inline void     CenterHandle() { SetHandle(width / 2, height / 2); }
        void            Render(float x, float y, float rot, float xscale, float yscale, const SScolorb &tint);
        inline void     Render(float x, float y, float rot, float xscale, float yscale) { Render(x, y, rot, xscale, yscale, tint); }
        inline void     Render(float x, float y, float rot, float scale) { Render(x, y, rot, scale, scale, tint); }
        inline void     Render(float x, float y, float rot) { Render(x, y, rot, 1.0f, 1.0f, tint); }
        inline void     Render(float x, float y) { Render(x, y, 0.0f, 1.0f, 1.0f, tint); }

    private:
        void            Init(frameFlags f);
        inline void     Init() { Init((frameFlags)0); }
};

#endif

