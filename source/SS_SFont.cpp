/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_SFont.cpp
 *
 *  $Id: SS_SFont.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_SFont.h"

#include "SS_Utilities.h"
#include "SS_Files.h"
#include "SS_Game.h"
#include "SS_World.h"
#include "SS_LayerItem.h"

#include <SDL_image.h>
#include <SDL_keyboard.h>

#include <stdlib.h>
#include <string.h>

extern int SS_VIDEO_W, SS_VIDEO_H;

#define SS_SPACEFACTOR  3

//--------------------------------------------------------------
//  SS_World ... TextLayer methods
//--------------------------------------------------------------

//
// NewTextLayer(sfont, flags)
// NewTextLayer(sfont)
// NewTextLayer(filename, flags)
// NewTextLayer(filename)
//
// Create a text layer, set its sfont and flags, add it to the world, and return the layer
//
SS_TextLayer* SS_World::NewTextLayer(SS_SFont *sfont, Uint32 f)
{
    DEBUGF(1, "[%08X] SS_World::NewTextLayer(sfont, flags)\n", this);

    SS_TextLayer *layer = new SS_TextLayer(sfont, f);
    AddLayer(layer);
    return layer;
}

SS_TextLayer* SS_World::NewTextLayer(SS_SFont *sfont)
{
    DEBUGF(1, "[%08X] SS_World::NewTextLayer(sfont)\n", this);

    SS_TextLayer *layer = new SS_TextLayer(sfont);
    AddLayer(layer);
    return layer;
}

SS_TextLayer* SS_World::NewTextLayer(char *filename, float desc, Uint32 f)
{
    DEBUGF(1, "[%08X] SS_World::NewTextLayer(\"%s\")\n", this, filename);

    return NewTextLayer(new SS_SFont(filename, desc), f);
}

SS_TextLayer* SS_World::NewTextLayer(char *filename, float desc)
{
    DEBUGF(1, "[%08X] SS_World::NewTextLayer(\"%s\")\n", this, filename);

    return NewTextLayer(new SS_SFont(filename, desc));
}



#pragma mark -
//--------------------------------------------------------------
//  SS_SFont
//  An SFont that can be used to print stuff
//--------------------------------------------------------------

SS_SFont::SS_SFont(char *filename, float desc)
{
    DEBUGF(1, "[%08X] SS_SFont::SS_SFont(\"%s\")\n", this, filename);

    height      = 0.0;
    descender   = 0.0;

    LoadFont(filename, desc);
}

SS_SFont::~SS_SFont()
{
    if (height != 0.0)
        glDeleteTextures(SS_CHR_COUNT, gl_texture);
}

//
// LoadFont
// Load an SFont file and create textures for every character
//
void SS_SFont::LoadFont(char *filename, float desc)
{
    DEBUGF(1, "[%08X] SS_SFont::LoadFont(\"%s\")\n", this, filename);

    Uint16      x, y, chr;
    Uint16      wide;
    bool        pixon, oldpixon = true;
    SDL_Rect    charRect;

    SDL_Surface *rawfont;
    char        *full = SS_Folder::FullPath(filename);

    if (!(rawfont = IMG_Load(full))) {
        printf("%s\n", SDL_GetError());
        throw "Can't Load SFont image file.";
    }

    //
    // The top-left pixel provides the marker color
    //
    Uint32 markerColor = SS_RGBA(::GetPixel(rawfont, 0, 0));

    //
    // Purge old textures associated with this SFont
    //
    if (height != 0.0)
        glDeleteTextures(SS_CHR_COUNT, gl_texture);

    //
    // Count the markers - hence the lines of characters - from top to bottom
    //
    Uint16  lastY = 0;
    Uint16  lineHeight = 0;
    Uint16  markerCount = 1;
    for (y=1; y < rawfont->h; y++)
    {
        // Check for a marker pixel (or not)
        pixon = SS_RGBA(::GetPixel(rawfont, 0, y)) == markerColor;
        if (pixon)
        {
            // The distance from the last marker to this one
            Uint16  h = y - lastY - 1;

            // If the distance is different from a previous instance then bail
            if (lineHeight != 0 && lineHeight != h)
                throw "SFont corrupt! Line heights inconsistent.";

            // Remember the height of a line, and where the marker was
            lineHeight = h;
            lastY = y;

            // Count up a marker
            markerCount++;
        }
    }

    // Remember the full height for use in texture coordinates
    fullHeight = rawfont->h - 1.0;

    // If only one line of characters then a line height is all minus the marker
    if (lineHeight == 0)
        lineHeight = rawfont->h - 1.0;

    // Initialize a single character rect
    charRect.y = 1;
    height = lineHeight;
    charRect.h = lineHeight;

    // Start with character index 0
    chr = 0;

    // The first marker line is always 0
    Uint16 markerLine = 0;

    // The start of a given character
    Uint16 start = 0;

    // Loop through all the marked lines
    for (int line=markerCount; line--;)
    {
        // The line will begin just below the marker
        charRect.y = markerLine + 1;

        // And all the way across left to right
        for (x=0; x < rawfont->w; x++)
        {
            // Check for a marker pixel transition
            pixon = SS_RGBA(::GetPixel(rawfont, x, markerLine)) == markerColor;
            if (pixon != oldpixon)
            {
                // Got a transition, remember it
                oldpixon = pixon;

                // Transitioned out of a character? (off-to-on)
                if (pixon)
                {
                    // distance from the previous on-to-off transition to here
                    wide = x - start;

                    // Complete the texture rectangle for the character
                    charRect.x = start;
                    charRect.w = wide;

                    // Make the character texture and get back the resulting 2^z texture width & height
                    if (!::MakeTextureFromSurface(rawfont, &charRect, &gl_texture[chr], &texw[chr], &texh))
                        throw "OpenGL texture could not be created.";

                    // Remember some helpful statistics for drawing the texture
                    width[chr] = wide;
                    wfactor[chr] = (float)wide / texw[chr];

                    // printf("Character %02d [ %03d %03d ] [ %03d %03d ]\n", chr, charRect.x, charRect.y, charRect.w, charRect.h);

                    // Found all the characters we have room for?
                    if (++chr >= SS_CHR_COUNT)
                        break;
                }
                else
                {
                    // store the start for an on-to-off transition
                    start = x;
                }
            }
        }

        // Move to the next line down
        markerLine += lineHeight + 1;
    }

    printf("\n\n\n");

    hfactor = height / texh;
    SetSpacing(2, 2);
    descender = desc;

    SDL_FreeSurface(rawfont);
}

//
// Render
// Render a character at the given screen coordinates
//
void SS_SFont::Render(char chr, float x, float y, SScolorb *inTint, SDL_Rect *bounds)
{
    float   xscale = 1.0, yscale = 1.0, rot = 0.0;
    Uint16  c = chr - SS_FIRST_CHR;
    float   w = width[c] * xscale;
    float   h = height * yscale;
    SScolorb *t;

    if (inTint != NULL)
        t = inTint;
    else
    {
        SScolorb white = { 255, 255, 255, 255 };
        t = &white;
    }

    gl_do_texture(1);
    gl_do_blend(1);
    gl_bind_texture(gl_texture[c]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Clip against the received bounds
    //
    if (bounds)
        ClipRectangle(bounds);

    // Translate
    //
    glTranslatef(x, y - Ascent(), 0.0);

    // Rotate
    //
//  glRotatef(rot, 0.0, 0.0, 1.0);


    // TODO: Generate display lists for all characters
    //  with multiple textures - and try to make fewer
    //  textures
    //

    glBegin(GL_QUADS);
    glColor4ubv((GLubyte*)t);
    glTexCoord2f(0.0, 0.0);             glVertex2f(0, 0);
    glTexCoord2f(wfactor[c], 0.0);      glVertex2f(w, 0);
    glTexCoord2f(wfactor[c], hfactor);  glVertex2f(w, h);
    glTexCoord2f(0.0, hfactor);         glVertex2f(0, h);
    glEnd();

    // Turn off the clipping
    //
    if (bounds)
        ClipRectangle(NULL);

    glPopMatrix();
}

//
// Render(text, x, y, color)
// Render a string of characters at the given screen coordinates
//
void SS_SFont::Render(char *text, float x, float y, SScolorb *inTint, SDL_Rect *bounds)
{
    char        c;
    float       left = 0;

    while ((c = *text++))
    {
        if (c >= SS_FIRST_CHR)
        {
            Render(c, x + left, y, inTint, bounds);
            left += width[c - SS_FIRST_CHR] + xspace;
        }
        else if (c == 13 || c == 10)
        {
            left = 0;
            y += height + yspace;
        }
        else if (c == 32)
            left += xspace * SS_SPACEFACTOR;
    }
}

//
// UpdateCursor
// Move the cursor to the position it would be at after printing a given string
//
void SS_SFont::UpdateCursor(char *text, float *x, float *y)
{
    char    c;
    float   left = 0;

    while ((c = *text++))
    {
        if (c >= SS_FIRST_CHR)
            left += width[c - SS_FIRST_CHR] + xspace;
        else if (c == 13 || c == 10) {
            left = 0;
            *y += height + yspace;
        }
        else if (c == 32)
            left += xspace * SS_SPACEFACTOR;
    }

    *x += left;
}

//
// StringWidth
// Return the pixel width of a given string
//
float SS_SFont::StringWidth(char *text, Sint16 pos)
{
    char    c;
    float   wide = 0, left = 0;
    Uint16  i = 0;

    while ((c = *text++) && (pos < 0 || pos > i++))
    {
        if (c >= SS_FIRST_CHR)
            left += width[c - SS_FIRST_CHR] + xspace;
        else if (c == 13 || c == 10)
            left = 0;
        else if (c == 32)
            left += xspace * SS_SPACEFACTOR;

        if (left > wide)
            wide = left;
    }

    if (wide)
        wide -= xspace;

    return (Uint16)wide;
}

//
// IndexOfPoint
// Return the index in the string for the given local coordinate
//
Uint16 SS_SFont::IndexOfPoint(char *text, float pixx, float pixy)
{
    char    c;
    Uint16  i = 0, ind = strlen(text);
    float   xpos = 0, ypos = 0, wide, spc;
    float   h = height + yspace;

    if (pixy < 0)
        pixy = pixx = 0;
    else if (pixx < 0)
        pixx = 0;

    while ((c = text[i]))
    {
        if (c >= SS_FIRST_CHR) {
            wide = width[c - SS_FIRST_CHR];
            spc = xspace;
        }
        else if (c == 13 || c == 10) {
            xpos = 0;
            ypos += h;
            wide = spc = 0;
        }
        else if (c == 32) {
            wide = xspace * SS_SPACEFACTOR;
            spc = 0;
        }

        if (wide && (pixy - ypos < h) && (pixx - xpos < wide))
        {
            ind = i + ((pixx - xpos) > (wide / 2) ? 1 : 0);
            break;
        }

        xpos += wide + spc;
        i++;
    }

    return ind;
}

//
// IsPointInside
// Return whether a point is inside some text
//
bool SS_SFont::IsPointInside(char *text, float pixx, float pixy)
{
    return ( pixx >= 0
        && pixx < StringWidth(text)
        && pixy >= -Ascent()
        && pixy < descender );
}



#pragma mark -
//--------------------------------------------------------------
//  SS_String
//  A bit of text associated with a font and stuff
//--------------------------------------------------------------

SS_String::SS_String(SS_SFont *font, char *t, float x, float y)
{
    DEBUGF(1, "[%08X] SS_String(font, text, x, y) CONSTRUCTOR\n", this);

    Init();

    sfont = font;
    drawx = xpos = x;
    drawy = ypos = y;

    SetText(t);
    RedoAlignment();
}

SS_String::SS_String(SS_SFont *font)
{
    DEBUGF(1, "[%08X] SS_String(font) CONSTRUCTOR\n", this);

    Init();

    sfont = font;
}

SS_String::~SS_String()
{
    if (text)
        delete [] text;
}

//
// Init
//
void SS_String::Init()
{
    DEBUGF(1, "[%08X] SS_String::Init()\n", this);

    sfont           = NULL;
    text            = NULL;
    alignment       = SA_LEFT;
    xpos            = 0;
    ypos            = 0;
    drawx           = 0;
    drawy           = 0;

    SetTint(255, 255, 255, 255);
    SetText("");
}

//
// SetText(text)
//
void SS_String::SetText(const char *t)
{
    DEBUGF(1, "[%08X] SS_String::SetText(%s)\n", this, t);

    if (text) delete [] text;
    text = t ? newstring(t) : NULL;
}

//
// AlignToRect
//
void SS_String::AlignToRect(SS_Rect &rect, stringAlign position)
{
    DEBUGF(1, "[%08X] SS_String::AlignToRect(%08X, {%.2f, %.2f, %.2f, %.2f}, %X)\n", this, rect.x, rect.y, rect.w, rect.h, position);

    float       x, y;
    int a = 0;

    switch (position & SA_HMASK)
    {
        case 0:
        case SA_LEFT:
            x = rect.x + rect.w + 2;
            a = SA_LEFT;
            break;

        case SA_RIGHT:
            x = rect.x - 2;
            a = SA_RIGHT;
            break;

        case SA_CENTERH:
            x = rect.x + rect.w / 2;
            a = SA_CENTERH;
            break;
    }

    switch (position & SA_VMASK)
    {
        case 0:
        case SA_BASELINE:
            y = rect.y + rect.h;
            a |= SA_BASELINE;
            break;

        case SA_BELOW:
            y = rect.y + rect.h + 2;
            a |= SA_BELOW;
            break;

        case SA_CENTERV:
            y = rect.y + rect.h / 2;
            a |= SA_CENTERV;
            break;

        case SA_ABOVE:
            y = rect.y - 2;
            a |= SA_ABOVE;
            break;
    }

    Move(x, y);
    SetAlignment((stringAlign)a);
}

//
// AlignToSprite
//
void SS_String::AlignToSprite(SS_LayerItem *spr, stringAlign position)
{
    DEBUGF(1, "[%08X] SS_String::AlignToSprite(%08X, %d)\n", this, spr, position);

    SS_Point point;
    spr->LocalPosition(&point);

    SS_Rect rect = {
        point.x - spr->xhandle,
        point.y - spr->yhandle,
        spr->width,
        spr->height
    };

    AlignToRect(rect, position);
}

//
// RedoAlignment
//
// Based on the string size and alignment decide where the x and baseline are.
// The result is stored in (drawx, drawy) for rendering purposes.
// The draw coordinates naturally also affect point testing.
//
void SS_String::RedoAlignment()
{
    DEBUGF(1, "[%08X] SS_String::RedoAlignment()\n", this);

    // Determine where the left-edge should be positioned
    switch (alignment & SA_HMASK)
    {
        case 0:
        case SA_LEFT:
            drawx = xpos;
            break;

        case SA_RIGHT:
            drawx = xpos - Width();
            break;

        case SA_CENTERH:
            drawx = xpos - Width() / 2;
            break;
    }

    // Determine where the baseline should be positioned
    switch (alignment & SA_VMASK)
    {
        case 0:
        case SA_BASELINE:
            drawy = ypos;
            break;

        case SA_ABOVE:
            drawy = ypos - Descender();
            break;

        case SA_CENTERV:
            drawy = ypos + Ascent() / 2;
            break;

        case SA_BELOW:
            drawy = ypos + Ascent();
            break;
    }
}

//
// StringCopy(text)
// Copy a string directly into the string buffer
//
void SS_String::StringCopy(const char *t)
{
    DEBUGF(1, "[%08X] SS_String::StringCopy(%s)\n", this, t);

    if (text)
        strcpy(text, t);
}

//
// SetWithFloat(float)
//
void SS_String::SetWithFloat(const float f)
{
    char *temp;
    if (asprintf(&temp, "%.2f", f) != -1)
    {
        SetText(temp);
        free(temp);
    }
}

//
// SetWithInt(int)
//
void SS_String::SetWithInt(const int i)
{
    char *temp;
    if (asprintf(&temp, "%d", i) != -1)
    {
        SetText(temp);
        free(temp);
    }
}

//
// SetHex(int, digits)
//
void SS_String::SetHex(const int h)
{
    if (text)
        sprintf(text, "%08X", h);
}

//
// Render(tint, rect)
//
//  TODO: Make it possible to Freeze() various containers
//  in such a way that they still can be moved and rotated
//  without a new DL (e.g., impose external transforms from
//  outside and maintain the rendered characters in the
//  local coordinate space of the string.)
//
void SS_String::Render(SScolorb &inTint, SDL_Rect *rect)
{
    SScolorb outTint;
    MultiplyColorQuads(inTint, tint, outTint);
    sfont->Render(text, drawx, drawy, &outTint, rect);
}


#pragma mark -
//--------------------------------------------------------------
//  SS_EditString
//  A string with an associated selection
//--------------------------------------------------------------

SS_EditString::SS_EditString(SS_SFont *font) : SS_String(font)
{
    DEBUGF(1, "[%08X] SS_EditString(font) CONSTRUCTOR\n", this);

    Init();
}

SS_EditString::SS_EditString(SS_SFont *font, char *t, float x, float y) : SS_String(font, t, x, y)
{
    DEBUGF(1, "[%08X] SS_EditString(font, t, x, y) CONSTRUCTOR\n", this);

    Init();
}

SS_EditString::~SS_EditString()
{
}

//
// Init
//
void SS_EditString::Init()
{
    DEBUGF(1, "[%08X] SS_EditString::Init()\n", this);

    maxLength       = 0;        // no limit
    maxWidth        = 0;        // no limit
    hasFocus        = false;
    selectionStart  = 0;
    selectionLength = 0;
    cursStyle       = CURS_UNDERLINE;

    SetCursorTint1(0, 128, 128);
    SetCursorTint2(0, 192, 192);
    SetSelectionTint(0, 0, 128);
}

//
// CursorLeft(ext)
// Move cursor left, optionally extending the selection
//
void SS_EditString::CursorLeft(bool ext)
{
    Sint16  s = selectionStart;
    Sint16  l = selectionLength;

    if (ext)
        SetSelection(s, l - 1);
    else {
        if (l < 0) { s += l; }
        SetSelection(s - (s == 0 ? 0 : 1));
    }
}

//
// CursorRight(ext)
// Move cursor right, optionally extending the selection
//
void SS_EditString::CursorRight(bool ext)
{
    Sint16  s = selectionStart;
    Sint16  l = selectionLength;

    if (ext)
        SetSelection(s, l + 1);
    else
        SetSelection(s + (l > 1 ? l : 1));
}

//
// CursorHome(ext)
// Move cursor home, optionally extending the selection
//
void SS_EditString::CursorHome(bool ext)
{
    Sint16  s = selectionStart;

    if (ext)
        SetSelection(s, -s);
    else
        SetSelection(0);
}

//
// CursorEnd(ext)
// Move cursor to end, optionally extending the selection
//
void SS_EditString::CursorEnd(bool ext)
{
    Uint16  len = Length();

    if (ext)
    {
        Sint16  s = selectionStart;
        SetSelection(s, len - s);
    }
    else
        SetSelection(len);
}

//
// CursorWordLeft(ext)
// Move cursor left one word, optionally extending the selection
//
void SS_EditString::CursorWordLeft(bool ext)
{
    Sint16  s = selectionStart, st = s, os;
    Sint16  l = selectionLength;
    bool    non = false;

    if (ext || l < 0) { st += l; }

    os = st;
    while (st > 0)
    {
        if (text[--st] > SDLK_SPACE)
            non = true;
        else if (non) {
            st++;
            break;
        }
    }

    if (ext)
        SetSelection(s, l + st - os);
    else
        SetSelection(st);
}

//
// CursorWordRight(ext)
// Move cursor right, optionally extending the selection
//
void SS_EditString::CursorWordRight(bool ext)
{
    Sint16  s = selectionStart, st = s, os;
    Sint16  l = selectionLength;
    bool    non = false;

    if (ext || l > 0) { st += l; }

    os = st;
    while (st < Length())
    {
        if (text[++st] > SDLK_SPACE)
            non = true;
        else if (non)
            break;
    }

    if (ext)
        SetSelection(s, l + st - os);
    else
        SetSelection(st);
}

//
// SelectWord(start)
//
void SS_EditString::SelectWord(Uint16 i, bool ext)
{
    Uint16  len = Length();
    if (len == 0) return;

    if (i > len - 1) i = len - 1;

    bool    neg = (text[i] <= SDLK_SPACE);
    Sint16  st = i, en = i;
    char    c;

    while(st >= 0)
    {
        c = text[st];
        if (neg ? (c > SDLK_SPACE) : (c <= SDLK_SPACE)) {
            st++;
            break;
        }
        st--;
    }

    if (st < 0) st = 0;

    while((c = text[en]))
    {
        if (neg ? (c > SDLK_SPACE) : (c <= SDLK_SPACE)) {
            en--;
            break;
        }
        en++;
    }

    if (ext) {
        if (st > wordStart) st = wordStart;
        if (en < wordEnd) en = wordEnd;
    }
    else {
        wordStart = st;
        wordEnd = en;
    }

    SetSelection(st, en - st + 1);
}

//
// SetSelection(start, len)
//
void SS_EditString::SetSelection(Sint16 s, Sint16 l)
{
    DEBUGF(1, "[%08X] SS_EditString::SetSelection(s, l)\n", this);

    Uint16  len = Length();

    if (s < 0) s = 0;                   // aucun commencer en-dessous de 0
    if (s + l < 0) l = -s;              // aucun choix en-dessous de 0
    if (s > len) { s = len; l = 0; }    // aucun commencer après l'extrémité
    if (s + l > len) l = len - s;       // aucun choix après l'extrémité

    selectionStart = s;
    selectionLength = l;
}

//
// ReplaceSelection(text)
//
void SS_EditString::ReplaceSelection(const char *t)
{
    DEBUGF(1, "[%08X] SS_EditString::ReplaceSelection(t)\n", this);

    Uint16  s = selectionStart;
    Sint16  l = selectionLength;
    Uint16  tlen = strlen(t);

    if (l < 0) { s += l; l = -l; }

    Uint16  len = Length() + tlen - l;
    char    *newStr = new char[len + 1];

    // Copy string up to the selection
    if (s)
        strncpy(newStr, text, s);

    // Append the insertion string
    strcpy(&newStr[s], t);

    // Append everything after the selection
    strcpy(&newStr[s + tlen], &text[s + l]);

    if ((maxWidth == 0 || sfont->StringWidth(newStr) <= maxWidth) && (maxLength == 0 || len <= maxLength))
    {
        SetText(newStr);
        SetSelection(s + (tlen ? 1 : 0));
    }

    delete [] newStr;
}

//
// ReplaceSelection(char)
//
void SS_EditString::ReplaceSelection(char c)
{
    char newStr[] = { c, 0 };
    ReplaceSelection(newStr);
}

//
// HandleKey(key)
//
void SS_EditString::HandleKey(const SDL_Keysym &key)
{
    if (key.mod & (KMOD_ALT|KMOD_CTRL)) return;

    Sint16  s = selectionStart;
    Sint16  l = selectionLength;

    if (l < 0) { s += l; l = -l; }

    switch (key.sym)
    {
        case SDLK_DELETE:
            if (s < Length() && l == 0) SetSelection(s, 1);
            ReplaceSelection("");
            break;

        case SDLK_BACKSPACE:
            if (s > 0 && l == 0) SetSelection(s - 1, 1);
            ReplaceSelection("");
            break;

        case SDLK_UP:
        case SDLK_HOME:
            CursorHome(key.mod & KMOD_SHIFT);
            break;

        case SDLK_DOWN:
        case SDLK_END:
            CursorEnd(key.mod & KMOD_SHIFT);
            break;

        case SDLK_LEFT:
            if (key.mod & KMOD_ALT)
                CursorWordLeft(key.mod & KMOD_SHIFT);
            else
                CursorLeft(key.mod & KMOD_SHIFT);
            break;

        case SDLK_RIGHT:
            if (key.mod & KMOD_ALT)
                CursorWordRight(key.mod & KMOD_SHIFT);
            else
                CursorRight(key.mod & KMOD_SHIFT);
            break;

        default:
            if (key.sym == ' ' || (key.sym >= SS_FIRST_CHR && key.sym <= SS_LAST_CHR))
                ReplaceSelection(key.sym);
    }
}

//
// Render
//
void SS_EditString::Render(const SScolorb &inTint, SDL_Rect *bounds)
{
    DEBUGF(1, "[%08X] SS_EditString::Render()\n", this);

    SScolorb outTint;
    MultiplyColorQuads(inTint, tint, outTint);

    RenderCursor(inTint, bounds);
    SS_String::Render(outTint, bounds);
}

//
// RenderCursor
// Draw the selection or cursor
//
void SS_EditString::RenderCursor(const SScolorb &inTint, SDL_Rect *bounds)
{
    DEBUGF(1, "[%08X] SS_EditString::RenderCursor()\n", this);

    if (bounds)
        ClipRectangle(bounds);

    Uint16      s = selectionStart;
    Sint16      l = selectionLength;
    cursorStyle c = cursStyle;
    float       w;
    float       o = sfont->XSpace() / 2;
    SScolorb    outTint;

    if (l < 0) { s += l; l = -l; }

    GLfloat x1 = xpos + Width(s) + (s ? o : -o);
    GLfloat y2 = Bottom();
    GLfloat x2, y1;

    if (l)
    {
        MultiplyColorQuads(inTint, selectionTint, outTint);
        x2 = xpos + Width(s + l);
        y1 = Top();

        gl_do_texture(0);
        gl_do_blend(1);
        glMatrixMode(GL_MODELVIEW);
        glColor4ubv((GLubyte*)&outTint);

        if (hasFocus)
            glRectf(x1, y1, x2, y2);
        else
        {
            gl_line_width(1);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x1, y1);
            glVertex2f(x2, y1);
            glVertex2f(x2, y2);
            glVertex2f(x1, y2);
            glEnd();
        }
    }
    else if (hasFocus)
    {
        s = selectionStart;
        outTint = (SDL_GetTicks() & 0x100) ? cursorTint1 : cursorTint2;

        switch(c)
        {
            case CURS_UNDERLINE:
                w = Width(s + 1);
                if (s >= Length())
                    w += sfont->CharacterWidth('t');
                y1 = y2 - 3;
                x2 = xpos + w + o;
                break;

            case CURS_BLOCK:
                w = Width(s + 1);
                if (s >= Length())
                    w += sfont->CharacterWidth('t');
                y1 = Top();
                x2 = xpos + w + o;
                break;

            case CURS_BAR:
                y1 = Top();
                x2 = x1 + 1;
                break;
        }

        gl_do_texture(0);
        gl_do_blend(1);
        glMatrixMode(GL_MODELVIEW);
        glColor4ubv((GLubyte*)&outTint);
        glRectf(x1, y1, x2, y2);
    }

    if (bounds)
        ClipRectangle(NULL);
}

#pragma mark -
//--------------------------------------------------------------
//  SS_TextLayer
//  A layer containing string objects
//--------------------------------------------------------------

SS_TextLayer::SS_TextLayer(char *filename, float desc) : SS_Layer(SS_NOSCROLL|SS_NOZOOM)
{
    Init();
    SetBaseFont(new SS_SFont(filename, desc), true);
}

SS_TextLayer::SS_TextLayer(char *filename, float desc, Uint32 f) : SS_Layer(f)
{
    Init();
    SetBaseFont(new SS_SFont(filename, desc), true);
}

SS_TextLayer::SS_TextLayer(SS_SFont *font) : SS_Layer(SS_NOSCROLL|SS_NOZOOM)
{
    Init();
    SetBaseFont(font);
}

SS_TextLayer::SS_TextLayer(SS_SFont *font, Uint32 f) : SS_Layer(f)
{
    Init();
    SetBaseFont(font);
}

SS_TextLayer::~SS_TextLayer()
{
    Clear();

    if (isMyFont)
        delete basefont;
}

//
// Init
//
void SS_TextLayer::Init()
{
    SDL_Rect whole = { 0, 0, SS_VIDEO_W, SS_VIDEO_H };

    basefont    = NULL;     // No font yet set
    runfont     = NULL;     // No running font set
    xcurs       = 0;
    ycurs       = 0;
    bounds      = whole;
}

//
// CenterCursor
//
void SS_TextLayer::CenterCursor()
{
    if (flags & SS_NOZOOM) {
        xcurs = world->ViewWidth() / 2;
        ycurs = (world->ViewHeight() / 2) - (runfont->Ascent() / 2);
    }
    else {
        xcurs = world->ZoomWidth() / 2;
        ycurs = (world->ZoomHeight() / 2) - (runfont->Ascent() / 2);
    }
}

//
// CenterCursorX
//
void SS_TextLayer::CenterCursorX()
{
    xcurs = ((flags & SS_NOZOOM) ? world->ViewWidth() : world->ZoomWidth()) / 2;
}

//
// CenterCursorY
//
void SS_TextLayer::CenterCursorY()
{
    ycurs = (((flags & SS_NOZOOM) ? world->ViewHeight() : world->ZoomHeight()) / 2) - (runfont->Height() / 2);
}

//
// Print(font, text, x, y)
//
SS_String* SS_TextLayer::Print(SS_SFont *font, char *text, float x, float y)
{
    SS_String   *string = new SS_String(font, text, x, y);
    AddString(string);

    runfont = font;
    xcurs = x; ycurs = y;
    runfont->UpdateCursor(text, &xcurs, &ycurs);

    return string;
}

//
// Render
//
void SS_TextLayer::Render()
{
    DEBUGF(1, "[%08X] SS_TextLayer::Render()\n", this);

    PrepareMatrix();

    SS_String   *item;
    SS_ItemIterator itr = GetIterator();
    while ((item = (SS_String*)itr.NextItem()))
        item->Render(tint, &bounds);
}

