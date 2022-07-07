/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Vectors.h
 *
 *  $Id: SS_Vectors.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_VECTORSPRITE_H__
#define __SS_VECTORSPRITE_H__

#include <OpenGL/glu.h>

#include "SS_RefCounter.h"
#include "SS_LayerItem.h"
#include "SS_Layer.h"
#include "SS_Templates.h"

enum SSVectorType {
    SS_NOTYPE = 0,
    SS_POINTS,                  // n points
    SS_LINES,                   // n/2 lines
    SS_LINE_STRIP,              // n-1 lines
    SS_LINE_LOOP,               // n lines (closed outline polygon)
    SS_TRIANGLES,               // n/3 triangles
    SS_TRIANGLE_STRIP,          // n-2 triangles (1-2-3, 2-3-4, 3-4-5...)
    SS_TRIANGLE_FAN,            // n-2 triangles (1-2-3, 1-3-4, 1-4-5...)
    SS_QUADS,                   // n/4 quadrilaterals
    SS_QUAD_STRIP,              // (n/2)-1 quadrilaterals
    SS_POLYGON,                 // n-sided closed solid polygon
    SS_OUTLINE_POLYGON,         // polygon outlined in another color
    SS_FRAME,                   // n/2 outlined rects
    SS_RECT,                    // n/2 filled rects
    SS_FRAME_RECT               // n/2 outlined and filled rects
};

#pragma mark -

//--------------------------------------------------------------
//
//  ssVector
//  A simple 2D vector
//
typedef struct { float x, y; } ssVector;

//--------------------------------------------------------------
//
//  SS_VectorArray
//  An array of vector structures
//
typedef TArray<ssVector> SS_VectorArray;

//--------------------------------------------------------------
//
//  SS_VectorUnit
//  A set of vectors of a common type
//
class SS_VectorUnit
{
    private:
        SSVectorType    m_mode;

    public:
        GLfloat         lineWeight;
        bool            useTint;
        SScolorb        lineTint;
        SScolorb        fillTint;

        SS_VectorArray  vectorList;

        SS_VectorUnit() { Init(); }

        SS_VectorUnit(const SSVectorType mode, const GLfloat w, bool uTint, const SScolorb &lTint, const SScolorb &fTint)
        {
            Init();
            m_mode      = mode;
            lineWeight  = w;
            useTint     = uTint;
            lineTint    = lTint;
            fillTint    = fTint;
        }

        SS_VectorUnit(const SS_VectorUnit &src)
        {
            *this = src;
        }

        void Init()
        {
            m_mode      = SS_NOTYPE;
            lineWeight  = 1.0f;
            useTint     = true;
            lineTint    = SS_WHITE_B;
            fillTint    = SS_BLUE_B;
        }

        const ssVector* operator[](unsigned i) { return &vectorList[i]; }

        inline SSVectorType     Mode()                  { return m_mode; }
        inline void         SetLineWeight(GLfloat w)    { lineWeight = w; }

        inline void         SetTintEnabled(bool b)      { useTint = b; }
        inline void         EnableTint()                { SetTintEnabled(true); }
        inline void         DisableTint()               { SetTintEnabled(false); }

        inline void         SetLineTint(SScolorb &t)    { lineTint = t; }
        inline void         SetFillTint(SScolorb &t)    { fillTint = t; }
        inline SScolorb&    LineTint()                  { return lineTint; }
        inline SScolorb&    FillTint()                  { return fillTint; }
        inline GLfloat      LineWeight()                { return lineWeight; }

        SS_VectorUnit&  operator=(const SS_VectorUnit &src)
        {
            if (&src != this)
            {
                m_mode      = src.m_mode;
                lineWeight  = src.lineWeight;
                lineTint    = src.lineTint;
                fillTint    = src.fillTint;
                vectorList  = src.vectorList;
            }

            return *this;
        }

};


#pragma mark -
//--------------------------------------------------------------
//
// SS_VectorUnitList
//

typedef TObjectArray<SS_VectorUnit> SS_VectorUnitList;


#pragma mark -
//--------------------------------------------------------------
// SS_VectorFrame
//
// A single static frame described by a list of vector lists.
// Each vector frame can contain several polygons of different
// colors and sizes.
//
class SS_VectorFrame : public SS_RefCounter
{
    protected:
        SS_VectorUnitList   unitList;               // unit list

        bool                antialias;              // antialias value sent during render
        GLfloat             lineWeight;             // default line weight for new units
        SScolorb            lineTint;               // default line tint for new units
        SScolorb            fillTint;               // default fill tint for new units
        bool                useTint;                // whether or not to apply tint as we go
        Sint16              begunUnit;              // the array being updated

        GLuint              gl_texture;             // an OpenGL texture for this thing
        GLuint              gl_list;                // the stored display list

    public:
        float               width, height;          // size of vector area
        float               smallx, smally, largex, largey;
        float               xhandle, yhandle;       // handle offset from center

                        SS_VectorFrame(GLfloat *vex, Uint16 len);
                        SS_VectorFrame(char *vectorFile);
                        SS_VectorFrame();
                        ~SS_VectorFrame();

        void            Begin(SSVectorType m=SS_POINTS, bool front=false);
        void            End();

        inline SS_VectorUnit* Unit()    { return (begunUnit > 0) ? unitList.Object(begunUnit-1) : NULL; }

        void            LoadImage(const char *filename);
        void            LoadSurface(SDL_Surface *s);
        void            SendGeometry(const SScolorb *inTint=&SS_WHITE_B);
        void            InitDisplayList();
        void            CalculateSize();

        inline void     GetHandle(float *x, float *y)   { *x = xhandle; *y = yhandle; }
        inline void     CenterHandle()                  { SetHandle(width / 2, height / 2); }
        inline void     SetHandle(float x, float y)     { xhandle = x; yhandle = y; InitDisplayList(); }

        void            PrependVector(float x, float y);
        void            AppendVector(float x, float y);
        void            PrependVectorList(SSVectorType mode, ssVector *list, Uint16 size);
        void            AppendVectorList(SSVectorType mode, ssVector *list, Uint16 size);
        void            PrependDuo(SSVectorType mode, float x1, float y1, float x2, float y2);
        void            AppendDuo(SSVectorType mode, float x1, float y1, float x2, float y2);

        void            PopFirst(Uint16 count);
        void            PopLast(Uint16 count);

        inline void     AppendLine(float x1, float y1, float x2, float y2)      { AppendDuo(SS_LINES, x1, y1, x2, y2); }
        inline void     PrependLine(float x1, float y1, float x2, float y2)     { PrependDuo(SS_LINES, x1, y1, x2, y2); }

        inline void     PrependBox(float x1, float y1, float x2, float y2)      { PrependDuo(SS_FRAME, x1, y1, x2, y2); }
        inline void     PrependBox(float w, float h)                            { PrependBox(-w/2, -h/2, w/2, h/2); }
        inline void     PrependBox(SS_Rect &rect)                               { PrependBox(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h); }
        inline void     AppendBox(float x1, float y1, float x2, float y2)       { AppendDuo(SS_FRAME, x1, y1, x2, y2); }
        inline void     AppendBox(float w, float h)                             { AppendBox(-w/2, -h/2, w/2, h/2); }
        inline void     AppendBox(SS_Rect &rect)                                { AppendBox(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h); }

        inline void     PrependRect(float x1, float y1, float x2, float y2)     { PrependDuo(SS_RECT, x1, y1, x2, y2); }
        inline void     PrependRect(float w, float h)                           { PrependRect(-w/2, -h/2, w/2, h/2); }
        inline void     PrependRect(SS_Rect &rect)                              { PrependRect(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h); }
        inline void     AppendRect(float x1, float y1, float x2, float y2)      { AppendDuo(SS_RECT, x1, y1, x2, y2); }
        inline void     AppendRect(float w, float h)                            { AppendRect(-w/2, -h/2, w/2, h/2); }
        inline void     AppendRect(SS_Rect &rect)                               { AppendRect(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h); }

        inline void     PrependRectBox(float x1, float y1, float x2, float y2)  { PrependDuo(SS_FRAME_RECT, x1, y1, x2, y2); }
        inline void     PrependRectBox(float w, float h)                        { PrependRectBox(-w/2, -h/2, w/2, h/2); }
        inline void     PrependRectBox(SS_Rect &rect)                           { PrependRectBox(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h); }
        inline void     AppendRectBox(float x1, float y1, float x2, float y2)   { AppendDuo(SS_FRAME_RECT, x1, y1, x2, y2); }
        inline void     AppendRectBox(float w, float h)                         { AppendRectBox(-w/2, -h/2, w/2, h/2); }
        inline void     AppendRectBox(SS_Rect &rect)                            { AppendRectBox(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h); }

        void            AppendRoundedBox(float x1, float y1, float x2, float y2, float r, int p=0, bool solid=false);
        void            AppendRoundedRect(float x1, float y1, float x2, float y2, float r, int p=0)     { AppendRoundedBox(x1, y1, x2, y2, r, p, false); }
        void            AppendRoundedRectBox(float x1, float y1, float x2, float y2, float r, int p=0)  { AppendRoundedBox(x1, y1, x2, y2, r, p, true); AppendRoundedBox(x1, y1, x2, y2, r, p, false); }

        void            AppendCircle(float x, float y, float r, int p=0, bool solid=false);
        inline void     AppendDisc(float x, float y, float r, int p=0) { AppendCircle(x, y, r, p, true); }
        void            AppendRing(float x, float y, float rinner, float router, int p);
        void            AppendStar(float x, float y, float r1, float r2, int p, bool solid=false);

        void            SetLineWeight(float w);
        inline void     Antialias(bool a=true)          { antialias = a; }

        inline void     SetTintEnabled(bool b)          { useTint = b; }
        inline void     EnableTint()                    { SetTintEnabled(true); }
        inline void     DisableTint()                   { SScolorb t = { 0xFF, 0xFF, 0xFF, 0xFF }; SetTintEnabled(false); SetLineTint(t); SetFillTint(t); }

        inline void     SetLineTint(const GLubyte &inTint)  { lineTint = (SScolorb&)inTint; }
        inline void     SetLineTint(const SScolorb &inTint) { lineTint = inTint; }
        inline void     SetLineTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { SScolorb t = { r, g, b, a }; lineTint = t; }
        inline void     SetLineTint(GLubyte r, GLubyte g, GLubyte b) { lineTint.r = r;  lineTint.g = g;  lineTint.b = b; }
        inline void     SetLineAlpha(GLubyte a) { lineTint.a = a; }

        inline void     SetFillTint(GLubyte *t) { fillTint = *(SScolorb*)t; }
        inline void     SetFillTint(SScolorb &inTint) { fillTint = inTint; }
        inline void     SetFillTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { fillTint.r = r;  fillTint.g = g;  fillTint.b = b;  fillTint.a = a; }
        inline void     SetFillTint(GLubyte r, GLubyte g, GLubyte b) { fillTint.r = r;  fillTint.g = g;  fillTint.b = b; }
        inline void     SetFillAlpha(GLubyte a) { fillTint.a = a; }

        void            Render(float x, float y, float rot, float xscale, float yscale, const SScolorb &inTint);
        void            Render(const SScolorb &inTint);
        void            Render();

        void            Tesselate();

        static void CALLBACK    vertexCallback(GLvoid *vertex, void *inData);
        static void CALLBACK    beginCallback(GLenum which, void *inData);
        static void CALLBACK    endCallback(void *inData);
        static void CALLBACK    errorCallback(GLenum *gl_error, void *inData);
        static void CALLBACK    combineCallback(GLdouble coords[3], GLdouble *vdata[4], GLfloat weight[4], GLdouble **outData, void *inData);

    private:
        void            Init();
};

#pragma mark -
//--------------------------------------------------------------
// SS_VectorSprite
//
// A single graphical object containing one or more frames
//
// Usage: Create "master sprite" instances then use sprite->Clone().
//

typedef TArray<SS_VectorFrame*> SS_VectorFrameArray;

#define SS_VECTOR_BLOCK 5
class SS_VectorSprite : public SS_Collider
{
    private:
        SS_VectorFrameArray     frameArray;     // An array to hold frame pointers

    public:
                        SS_VectorSprite();
                        SS_VectorSprite(const SS_VectorSprite &src) { Init(); *this = src; }
        virtual         ~SS_VectorSprite();

        virtual const SS_VectorSprite&  operator=(const SS_VectorSprite &src);
        virtual SS_VectorSprite*        Clone() { return new SS_VectorSprite(*this); }

        void            AddFrame(SS_VectorFrame *frame);
        inline void     AddFrame(char *vectorFile) { AddFrame(new SS_VectorFrame(vectorFile)); }

        void            ReleaseFrames();

        // Setters
        void            SetFrameIndex(Uint16 f) { currFrame = f % frameCount; /* frameArray[currFrame]->GetHandle(&xhandle, &yhandle); */ }
        void            SetAnimIncrement(Uint16 inc) { animIncrement = inc; }
        void            SetHandle(float x, float y);
        void            CenterHandle();

        // Diagnostics
        virtual bool    IsOnScreen();
        bool            TestPointCollision(float x, float y, bool isLocal);

        virtual void    Render(const SScolorb &inTint);

        Uint32          GetNewCollisions() { return 0x00000000; }

    private:
        void            Init();
};


#endif

