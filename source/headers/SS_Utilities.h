/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Utilities.h
 *
 *  $Id: SS_Utilities.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_UTILITIES_H__
#define __SS_UTILITIES_H__

#include <SDL.h>
#include "SS_Types.h"
#include "SS_Game.h"

// Utility Functions
bool InitSimpleSprite();
void GetTextureFromImage(const char* filename, float *w, float *h, GLuint *texture, Uint16 *tw, Uint16 *th, SDL_Surface **save_source=NULL, Uint16 flags=0);
bool MakeTextureFromSurface(SDL_Surface *source, SDL_Rect *rect, GLuint *texture, Uint16 *tw, Uint16 *th, Uint16 flags=0);
void ClipRectangle(SDL_Rect *bounds);
Uint32 GetPixel(SDL_Surface *surface, Sint16 x, Sint16 y);
void MakeAlphaForColor(SDL_Surface *surface, Uint32 rgb);
void AverageColorQuads(const SScolorb &one, const SScolorb &two, SScolorb &result);
void MultiplyColorQuads(const SScolorb &one, const SScolorb &two, SScolorb &result);
void RenderBox(float x, float y, float w, float h, const SScolorb &inTint, const colorSet *col);
void printSurfaceInfo(char *name, SDL_Surface *surface);
char* newstring(const char *str);
char* trim(char *str);
char* ltrim(char *str);
char* rtrim(char *str);

inline void GetAngularVelocity(float xv, float yv, float &a, float &v)
{
    a = SS_Game::AngleOfXY(xv, yv);
    v = SS_Game::Hypotenuse(xv, yv);
}

inline void GetCartesianVelocity(float a, float v, float &xv, float &yv)
{
    int i = SS_ROTINDEX(a + 270.0f);
    xv = SS_Game::Cos(i) * v;
    yv = SS_Game::Sin(i) * v;
}

extern SScolorb SS_WHITE_B;
extern SScolorb SS_BLACK_B;
extern SScolorb SS_RED_B;
extern SScolorb SS_GREEN_B;
extern SScolorb SS_BLUE_B;
extern SScolorb SS_YELLOW_B;
extern SScolorb SS_GRAY_B;
extern SScolorb SS_ALPHA50_B;

extern SScolorf SS_WHITE_F;
extern SScolorf SS_BLACK_F;
extern SScolorf SS_RED_F;
extern SScolorf SS_GREEN_F;
extern SScolorf SS_BLUE_F;
extern SScolorf SS_YELLOW_F;
extern SScolorf SS_GRAY_F;
extern SScolorf SS_ALPHA50_F;

#endif

