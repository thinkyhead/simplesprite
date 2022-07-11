/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Utilities.cpp
 *
 *  $Id: SS_Utilities.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Utilities.h"

#include "SS_Files.h"
#include "SS_Game.h"

#include <SDL_image.h>

SScolorb SS_WHITE_B     = { 0xFF, 0xFF, 0xFF, 0xFF };
SScolorb SS_BLACK_B     = { 0x00, 0x00, 0x00, 0xFF };
SScolorb SS_RED_B       = { 0xFF, 0x00, 0x00, 0xFF };
SScolorb SS_GREEN_B     = { 0x00, 0xFF, 0x00, 0xFF };
SScolorb SS_BLUE_B      = { 0x00, 0x00, 0xFF, 0xFF };
SScolorb SS_YELLOW_B    = { 0xFF, 0xFF, 0x00, 0xFF };
SScolorb SS_GRAY_B      = { 0x80, 0x80, 0x80, 0xFF };
SScolorb SS_ALPHA50_B   = { 0xFF, 0xFF, 0xFF, 0x80 };

SScolorf SS_WHITE_F     = { 1.0, 1.0, 1.0, 1.0 };
SScolorf SS_BLACK_F     = { 0.0, 0.0, 0.0, 1.0 };
SScolorf SS_RED_F       = { 1.0, 0.0, 0.0, 1.0 };
SScolorf SS_GREEN_F     = { 0.0, 1.0, 0.0, 1.0 };
SScolorf SS_BLUE_F      = { 0.0, 0.0, 1.0, 1.0 };
SScolorf SS_YELLOW_F    = { 1.0, 1.0, 0.0, 1.0 };
SScolorf SS_GRAY_F      = { 0.5, 0.5, 0.5, 1.0 };
SScolorf SS_ALPHA50_F   = { 1.0, 1.0, 1.0, 0.5 };

//--------------------------------------------------------------
//
// GetTextureFromImage
//
// Get an image file, make an OpenGL texture,
// and store the results in the passed pointers
//
void GetTextureFromImage(
    const char* filename,                   // image file
    float *outWidth,                        // width and height storage
    float *outHeight,
    GLuint *outTexture,                     // texture id storage
    Uint16 *outTxWidth,                     // texture-width and height storage
    Uint16 *outTxHeight,
    SDL_Surface **outSurface,               // (optional) surface storage
    Uint16 flags
    )
{
    #if SS_DEBUG
    printf("GetTextureFromImage()\n");
    #endif

    SDL_Surface *source;
    char        *full = SS_Folder::FullPath(filename);

    // Load up stuff for the tile map
    if (!(source = IMG_Load(full))) {
        printf("%s\n", SDL_GetError());
        throw "Can't Load image file.";
    }

//  printSurfaceInfo(filename, source);

    if (!::MakeTextureFromSurface(source, NULL, outTexture, outTxWidth, outTxHeight, flags))
        throw "OpenGL texture could not be created.";

    if (outWidth)   *outWidth = source->w;
    if (outHeight)  *outHeight = source->h;

    if (outSurface)
        *outSurface = source;       // Store the image pointer
    else
        SDL_FreeSurface(source);    // Or free the image
}

//--------------------------------------------------------------
//
// MakeTextureFromSurface
//
// Convert a surface into an OpenGL texture
// and store results in the passed pointers
//
bool MakeTextureFromSurface(
    SDL_Surface *source,        // surface to process
    SDL_Rect    *srcRect,       // (optional) region of the source
    GLuint      *outTexture,    // texture storage
    Uint16      *outTxWidth,    // (optional) texture width storage
    Uint16      *outTxHeight,   // (optional) texture height storage
    Uint16      flags           // (optional) flags
    )
{
    SDL_Surface *surface;
    SDL_Rect    sRect;  //, *srcRectPtr;
    Uint16      expw = 1, exph = 1;

    if (srcRect)
        sRect = *srcRect;
    else {
        sRect.x = 0;
        sRect.y = 0;
        sRect.w = source->w;
        sRect.h = source->h;
    }

    expw = sRect.w - 1;
    expw |= expw >> 1; expw |= expw >> 2; expw |= expw >> 4; expw |= expw >> 8; expw++;

    exph = sRect.h - 1;
    exph |= exph >> 1; exph |= exph >> 2; exph |= exph >> 4; exph |= exph >> 8; exph++;

    // find the smallest exponents that fit the image
//  while (expw < sRect.w) { expw <<= 1; }
//  while (exph < sRect.h) { exph <<= 1; }

    // Test whether OpenGL can handle this texture size
    GLint   width;
    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA8, expw, exph, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    if (width != expw) return false;

    // Create a 32-bit surface for a copy of the image
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, expw, exph, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    0xFF000000,
                    0x00FF0000,
                    0x0000FF00,
                    0x000000FF);
#else
                    0x000000FF,
                    0x0000FF00,
                    0x00FF0000,
                    0xFF000000);
#endif

    if(!surface)
        throw "Can't create Surface for image.";

//  printSurfaceInfo(NULL, surface);

    if (outTxWidth)     *outTxWidth = expw;
    if (outTxHeight)    *outTxHeight = exph;

    SDL_SetAlpha(source, 0, SDL_ALPHA_OPAQUE);          // Tell SDL to copy verbatim
    SDL_BlitSurface(source, &sRect, surface, NULL);

    glGenTextures(1, outTexture);                       // Generate one texture
    glBindTexture(GL_TEXTURE_2D, *outTexture);          // Bind it to the 2D Texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->w);    // Set row length to the width of the surface

    //
    // Antialias minimized and maximized textures with a linear method
    //
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (flags & SS_NO_BLEND_MAX) ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (flags & SS_NO_BLEND_MIN) ? GL_NEAREST : GL_LINEAR);

    //
    // Prevent edge pixels from bleeding
    //
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (flags & SS_BLEED_S) ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (flags & SS_BLEED_T) ? GL_REPEAT : GL_CLAMP);

    //
    // Hand the pixels to OpenGL
    //
    glTexImage2D(GL_TEXTURE_2D,     // 2D Texture image
        0,                          // Level of detail, 0=base
        GL_RGBA8,                   // The color data is RGBA8
        expw, exph,                 // Image size (2^m x 2^n, min. 64x64, +2 with border)
        0,                          // Border size (0 or 1)
        GL_RGBA,                    // Pixel format is RGBA
        GL_UNSIGNED_BYTE,           // Pixel's data type
        surface->pixels);           // The image data address

    glFlush();                      // Flush the gl buffer
    SDL_FreeSurface(surface);       // Free the surface

    return true;
}

//--------------------------------------------------------------
//
// MakeAlphaForColor
//
// Given a surface and a color generate a stencil in the alpha channel
//
void MakeAlphaForColor(SDL_Surface *surface, Uint32 rgb)
{
    Uint8       *ptr;
    Uint8       r = (rgb >> 16) & 0xFF;
    Uint8       g = (rgb >> 8) & 0xFF;
    Uint8       b = rgb & 0xFF;
    Uint32      i, size;

    ptr = (Uint8 *)surface->pixels;
    size = surface->w * surface->h * 4;

    for(i=0; i < size; i+=4)
    {
        if( (r == *ptr) && (g == *(ptr+1)) && (b == *(ptr+2)) )
            *(ptr+3) = 0;
        ptr += 4;
    }
}

//--------------------------------------------------------------
//
// GetPixel
//
Uint32 GetPixel(SDL_Surface *surface, Sint16 x, Sint16 y)
{
    Uint8   *bits;
    Uint16  bpp;
    Uint32  color;

    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return 0;

    bpp = surface->format->BytesPerPixel;

    SDL_LockSurface(surface);
    bits = ((Uint8*)surface->pixels) + (y * surface->pitch) + (x * bpp);
    SDL_UnlockSurface(surface);

    switch(bpp) {
        case 1:
            color = *((Uint8*)bits);
            break;

        case 2:
            color = *((Uint16*)bits);
            break;

        case 3:
        {
            Uint8 r = *(bits + surface->format->Rshift / 8);
            Uint8 g = *(bits + surface->format->Gshift / 8);
            Uint8 b = *(bits + surface->format->Bshift / 8);
            color = SDL_MapRGB(surface->format, r, g, b);

            if (surface->flags & SDL_SRCCOLORKEY) {
                if (surface->format->colorkey != color)
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    color = (color << 8) | 0x000000FF;  // rgba
#else
                    color |= 0xFF000000;                // abgr
#endif
            };
        }
            break;

        case 4:
            color = *((Uint32*)bits);
            break;

        default:
            color = 0;
            break;
   }

    return color;
}

//--------------------------------------------------------------
//
// AverageColorQuads
//
void AverageColorQuads(const SScolorb &one, const SScolorb &two, SScolorb &result)
{
    result.r = ((int)one.r + (int)two.r) / 2;
    result.g = ((int)one.g + (int)two.g) / 2;
    result.b = ((int)one.b + (int)two.b) / 2;
    result.a = ((int)one.a * (int)two.a) / 255;
}

//--------------------------------------------------------------
//
// MultiplyColorQuads
//
void MultiplyColorQuads(const SScolorb &one, const SScolorb &two, SScolorb &result)
{
    result.r = ((int)one.r * (int)two.r) / 255;
    result.g = ((int)one.g * (int)two.g) / 255;
    result.b = ((int)one.b * (int)two.b) / 255;
    result.a = ((int)one.a * (int)two.a) / 255;
}

//--------------------------------------------------------------
//
// RenderBox
//
void RenderBox(float x, float y, float w, float h, const SScolorb &inTint, const colorSet *col)
{
    SScolorb    fillTint;
    MultiplyColorQuads(col->fillColor, inTint, fillTint);

    if (fillTint.a)
    {
        gl_do_texture(0);
        gl_do_blend(1);
        glMatrixMode(GL_MODELVIEW);
        glColor4ubv((GLubyte*)&fillTint);
        glRectf(x, y, x + w, y + h);
    }

    float b = col->borderWeight;
    if (b)
    {
        SScolorb    borderTint;
        MultiplyColorQuads(col->borderColor, inTint, borderTint);

        if (borderTint.a)
        {
            gl_do_texture(0);
            gl_do_blend(1);
            gl_antialias(false);

            glMatrixMode(GL_MODELVIEW);
            glColor4ubv((GLubyte*)&borderTint);

            #if BORDERS_ARE_LINES

            gl_line_width(1);
            glBegin(GL_LINES);
            for (int q=1; q<=b; q++)
            {
        //          glRectf(x-q, y-q, x+w+q, y+h+q);
                glVertex2f(x-b, y-q+1);             // top-left
                glVertex2f(x+w+b, y-q+1);           // top-right

                glVertex2f(x-b, y+h+q);             // bottom-left
                glVertex2f(x+w+b, y+h+q);           // bottom-right

                glVertex2f(x+w+q, y);               // top-right
                glVertex2f(x+w+q, y+h);             // bottom-right

                glVertex2f(x-q+1, y);               // top-left
                glVertex2f(x-q+1, y+h);             // bottom-left
            }
            glEnd();

            #else

            glBegin(GL_QUAD_STRIP);

            glVertex2f(x-b, y-b);               // top-left-outside
            glVertex2f(x, y);                   // top-left-inside

            glVertex2f(x+w+b, y-b);             // top-right-outside
            glVertex2f(x+w, y);                 // top-right-inside

            glVertex2f(x+w+b, y+h+b);           // bottom-right-outside
            glVertex2f(x+w, y+h);               // bottom-right-inside

            glVertex2f(x-b, y+h+b);             // bottom-left-outside
            glVertex2f(x, y+h);                 // bottom-left-inside

            glVertex2f(x-b, y-b);               // top-left-outside
            glVertex2f(x, y);                   // top-left-inside

            glEnd();
            #endif
        }
    }
}

//--------------------------------------------------------------
//
// ClipRectangle
//
void ClipRectangle(SDL_Rect *bounds)
{
    if (bounds)
    {
        GLdouble clip0[] = { 0.0,  1.0, 0.0, double(-bounds->y) };              // top:     y >= bounds->y
        GLdouble clip1[] = { -1.0, 0.0, 0.0, double(bounds->x + bounds->w) };   // right:   bounds->x + bounds->w >= x
        GLdouble clip2[] = { 0.0, -1.0, 0.0, double(bounds->y + bounds->h) };   // bottom:  bounds->y + bounds->h >= y
        GLdouble clip3[] = {  1.0, 0.0, 0.0, double(-bounds->x) };              // left:    x >= bounds->x

        glClipPlane(GL_CLIP_PLANE0, clip0);
        glClipPlane(GL_CLIP_PLANE1, clip1);
        glClipPlane(GL_CLIP_PLANE2, clip2);
        glClipPlane(GL_CLIP_PLANE3, clip3);
        glEnable(GL_CLIP_PLANE0);
        glEnable(GL_CLIP_PLANE1);
        glEnable(GL_CLIP_PLANE2);
        glEnable(GL_CLIP_PLANE3);
    }
    else
    {
        glDisable(GL_CLIP_PLANE0);
        glDisable(GL_CLIP_PLANE1);
        glDisable(GL_CLIP_PLANE2);
        glDisable(GL_CLIP_PLANE3);
    }
}

//--------------------------------------------------------------
//
// printSurfaceInfo - For use in debugging
//
void printSurfaceInfo(char *name, SDL_Surface *surface)
{
    if (name)
        printf("Image: %s\n", name);

    printf("Size: %d x %d\n", surface->w, surface->h);
    printf("Flags: 0x%X\n", surface->flags);
    printf("Pitch: %d\n", surface->pitch);
    printf("BitsPerPixel: %d\n", surface->format->BitsPerPixel);
    printf("BytesPerPixel: %d\n", surface->format->BytesPerPixel);
    printf("Color Key: 0x%X\n", surface->format->colorkey);
    printf("Alpha: %d\n", surface->format->alpha);
    printf("Mask: %X %X %X %X\n\n", surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
}

//--------------------------------------------------------------
//
// newstring
//
char* newstring(const char *str)
{
    char *string = new char[strlen(str)+1];
    strcpy(string, str);
    return string;
}

//--------------------------------------------------------------
//
// trim - destructive string trim
//
char* trim(char *str)
{
    return (ltrim(rtrim(str)));
}

//--------------------------------------------------------------
//
// ltrim - string left trim
//
char* ltrim(char *str)
{
    char *s = str;

    while (*s != '\0' && (*s == '\t' || *s == ' '))
        s++;

    return s;
}

//--------------------------------------------------------------
//
// rtrim - destructive string right trim
//
char* rtrim(char *str)
{
    int i = strlen(str);

    while (i > 0 && (str[i-1] == '\t' || str[i-1] == ' '))
        i--;

    str[i] = '\0';

    return str;
}
