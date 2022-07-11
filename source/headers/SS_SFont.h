/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_SFont.h
 *
 *  $Id: SS_SFont.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_SFONT_H__
#define __SS_SFONT_H__

#include <SDL.h>
#include <SDL_opengl.h>

#include "SS_Types.h"
#include "SS_Layer.h"
#include "SS_LayerItem.h"

#include <string.h>

// Cursor Styles
enum cursorStyle {
    CURS_BAR,
    CURS_UNDERLINE,
    CURS_BLOCK
};

// String Proc Pointer
typedef void (*stringProcPtr)(SS_String*);

//--------------------------------------------------------------
// SS_SFont
// An OpenGL-based implementation of the SFont format
// SFont was originally created by Karl Bartel <karlb@gmx.net>
// Since his code is GPL/LGPL I had to roll my own version.
//
#define SS_FIRST_CHR 33
#define SS_LAST_CHR 127
#define SS_CHR_COUNT (SS_LAST_CHR - SS_FIRST_CHR + 1)

class SS_SFont
{
    private:
        GLuint          gl_texture[SS_CHR_COUNT];   // the texture names of each character
        Uint16          texw[SS_CHR_COUNT];     // texture pixel width
        Uint16          texh;                   // texture pixel height
        float           width[SS_CHR_COUNT];    // original pixel width
        float           wfactor[SS_CHR_COUNT];  // original width / texture width
        float           hfactor;                // original height / texture height
        float           xspace, yspace;         // space between characters / lines
        float           height;                 // original pixel height
        float           fullHeight;             // original full image height
        float           descender;              // the size of the dangly bits

    public:
                        SS_SFont(char *filename, float desc=0.0f);
                        ~SS_SFont();

        void            LoadFont(char *filename, float desc=0.0f);

        // Accessors
        inline float    XSpace() { return xspace; }
        inline float    YSpace() { return yspace; }
        inline float    Height() { return height; }
        inline float    Ascent() { return height - descender; }
        inline float    Descender() { return descender; }

        // Setters
        inline void     SetSpacing(float h, float v) { xspace = h; yspace = v; }
        inline void     SetDescender(float desc) { descender = desc; }
        inline void     SetBaseline(float base) { descender = height - base; }

        // Evaluations
        inline float    CharacterWidth(char chr) { return width[chr - SS_FIRST_CHR]; }
        inline float    StringWidth(char *text) { return StringWidth(text, -1); }
        float           StringWidth(char *text, Sint16 pos);

        bool            IsPointInside(char *text, float pixx, float pixy);
        Uint16          IndexOfPoint(char *text, float pixx, float pixy=0);

        // Rendering
        void            UpdateCursor(char *text, float *x, float *y);
        void            Render(char chr, float x, float y, SScolorb *tint=NULL, SDL_Rect *bounds=NULL);
        void            Render(char *text, float x, float y, SScolorb *tint=NULL, SDL_Rect *bounds=NULL);
};


#pragma mark -
//--------------------------------------------------------------
// SS_String
// A string suitable for use in an SS_TextLayer
//
// TODO:
//  - Add support for multi-line strings
//  - Add support for formatted strings
//
class SS_String : public SS_LayerItem
{
    protected:
        SS_SFont            *sfont;
        stringAlign         alignment;
        float               drawx, drawy;
        char                *text;

    public:
                            SS_String(SS_SFont *font);
                            SS_String(SS_SFont *font, char *t, float x, float y);
                            ~SS_String();

        virtual itemType    Type() { return SS_ITEM_STRING; }

        // Accessors
        inline char*        Text()              { return text; }
        inline Uint16       Length()            { return text ? strlen(text) : 0; }
        inline float        Width()             { return sfont->StringWidth(text); }
        inline float        Width(Uint16 pos)   { return sfont->StringWidth(text, pos); }
        inline float        Height()            { return sfont->Height(); }
        inline float        Ascent()            { return sfont->Ascent(); }
        inline float        Descender()         { return sfont->Descender(); }
        inline float        Top()               { return ypos - Ascent(); }
        inline float        Bottom()            { return ypos + Descender(); }

        // Setters
        inline void         SetFont(SS_SFont *font) { sfont = font; }
        void                SetText(const char *t);
        void                StringCopy(const char *t);
        void                SetWithFloat(const float f);
        void                SetWithInt(const int i);
        void                SetHex(const int h);

        // Evaluators
        inline bool         IsPointInside(float pixx, float pixy) { return sfont->IsPointInside(text, pixx - drawx, pixy - drawy); }
        inline Uint16       IndexOfPoint(float pixx, float pixy=0) { return sfont->IndexOfPoint(text, pixx + (drawx - xpos), pixy + (drawy - ypos)); }

        // Movement
        inline void         Move(float x, float y) { SS_LayerItem::Move(x, y); RedoAlignment(); }
        void                SetAlignment(stringAlign align) { alignment = align; RedoAlignment(); }
        void                AlignToRect(SS_Rect &rect, stringAlign position=(stringAlign)(SA_LEFT|SA_CENTERV));
        void                AlignToSprite(SS_LayerItem *spr, stringAlign position=(stringAlign)(SA_LEFT|SA_CENTERV));
        void                AlignToPoint(float x, float y, stringAlign position=(stringAlign)(SA_LEFT|SA_CENTERV));
        void                RedoAlignment();

        void                Render(SScolorb &inTint, SDL_Rect *rect=NULL);

    private:
        void                Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_EditString
// A string with a selection or cursor
//
class SS_EditString : public SS_String
{
    private:
        Uint16          maxLength;          // Maximum number of characters
        Uint16          maxWidth;           // Maximum pixel width
        Uint16          selectionStart;
        Sint16          selectionLength;
        Uint16          wordStart, wordEnd; // initial word selection
        bool            hasFocus;
        cursorStyle     cursStyle;
        SScolorb        cursorTint1;
        SScolorb        cursorTint2;
        SScolorb        selectionTint;

    public:
                            SS_EditString(SS_SFont *font);
                            SS_EditString(SS_SFont *font, char *t, float x, float y);
        virtual             ~SS_EditString();

        inline void         SetMaxStringLength(const Uint16 max) { maxLength = max; }
        inline void         SetMaxStringWidth(const Uint16 max) { maxWidth = max; }

        inline void         SetCursorStyle(const cursorStyle s) { cursStyle = s; }
        inline cursorStyle  CursorStyle() { return cursStyle; }

        inline void         SetCursorTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a=0xFF) { SetCursorTint1(r, g, b, a); SetCursorTint2(r, g, b, a); }
        inline void         SetCursorAlpha(GLubyte a) { SetCursorAlpha1(a); SetCursorAlpha2(a); }

        inline void         SetCursorTint1(GLubyte r, GLubyte g, GLubyte b, GLubyte a=0xFF) { cursorTint1.r = r;  cursorTint1.g = g;  cursorTint1.b = b;  cursorTint1.a = a; }
        inline void         SetCursorAlpha1(GLubyte a) { cursorTint1.a = a; }

        inline void         SetCursorTint2(GLubyte r, GLubyte g, GLubyte b, GLubyte a=0xFF) { cursorTint2.r = r;  cursorTint2.g = g;  cursorTint2.b = b;  cursorTint2.a = a; }
        inline void         SetCursorAlpha2(GLubyte a) { cursorTint2.a = a; }

        inline void         SetSelectionTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a=0xFF) { selectionTint.r = r;  selectionTint.g = g;  selectionTint.b = b;  selectionTint.a = a; }
        inline void         SetSelectionAlpha(GLubyte a) { selectionTint.a = a; }

        inline void         GainFocus() { hasFocus = true; }
        inline void         LoseFocus() { hasFocus = false; }

        void                CursorWordLeft(bool ext=false);
        void                CursorWordRight(bool ext=false);
        void                CursorLeft(bool ext=false);
        void                CursorRight(bool ext=false);
        void                CursorHome(bool ext=false);
        void                CursorEnd(bool ext=false);

        void                SelectWord(Uint16 i, bool ext=false);
        inline void         SelectAll() { SetSelection(0, Length()); }
        void                SetSelection(Sint16 s=0, Sint16 l=0);
        inline Uint16       SelectionStart() { return selectionStart; }
        inline Sint16       SelectionLength() { return selectionLength; }
        inline Uint16       SelectionEnd() { return selectionStart + selectionLength; }

        void                ReplaceSelection(const char *t);
        void                ReplaceSelection(char c);

        void                HandleKey(const SDL_Keysym &key);

        void                Render(const SScolorb &inTint, SDL_Rect *bounds=NULL);
        void                RenderCursor(const SScolorb &inTint, SDL_Rect *bounds=NULL);

    private:
        void                Init();
};


#pragma mark -
//--------------------------------------------------------------
// SS_TextLayer
// A layer that contains a list of stringSprite objects
//
class SS_TextLayer : public SS_Layer
{
    private:
        SS_SFont        *basefont, *runfont;
        bool            isMyFont;
        SDL_Rect        bounds;
        float           xcurs, ycurs;

    public:
                            SS_TextLayer(char *filename, float desc, Uint32 f);
                            SS_TextLayer(char *filename, float desc);
//                          SS_TextLayer(char *filename);
                            SS_TextLayer(SS_SFont *font, Uint32 f);
                            SS_TextLayer(SS_SFont *font);
                            ~SS_TextLayer();

        inline void         SetBaseFont(SS_SFont *font, bool own=false) { basefont=font; isMyFont=own; if (runfont == NULL) runfont = font; }
        inline void         SetRunFont(SS_SFont *font) { runfont = font; }
        void                AddString(SS_String *string) { AddItem(string); }
        void                MoveCursor(float x, float y) { xcurs = x; ycurs = y; }
        void                CenterCursor();
        void                CenterCursorX();
        void                CenterCursorY();
        SS_String*          Print(SS_SFont *font, char *text, float x, float y);
        inline SS_String*   Print(SS_SFont *font, char *text) { return Print(font, text, xcurs, ycurs); }
        inline SS_String*   Print(char *text, float x, float y) { return Print(runfont, text, x, y); }
        inline SS_String*   Print(char *text) { return Print(text, xcurs, ycurs); }
        inline SS_String*   PrintCenter(SS_SFont *font, char *text, float x, float y) { return Print(font, text, x - font->StringWidth(text) / 2, y); }
        inline SS_String*   PrintCenter(SS_SFont *font, char *text) { return PrintCenter(font, text, xcurs, ycurs); }
        inline SS_String*   PrintCenter(char *text, float x, float y) { return PrintCenter(runfont, text, x, y); }
        inline SS_String*   PrintCenter(char *text) { return PrintCenter(runfont, text, xcurs, ycurs); }

//      void                Clear();

//      void                Process();
        void                Animate() {}
        void                Render();

    private:
        void                Init();
};


#endif

