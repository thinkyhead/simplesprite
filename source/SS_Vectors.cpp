/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Vectors.cpp
 *
 *  $Id: SS_Vectors.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Vectors.h"
#include "SS_Game.h"
#include "SS_Utilities.h"

#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------
// SS_VectorFrame
//

SS_VectorFrame::SS_VectorFrame(GLfloat *vex, Uint16 len)
{
    Init();

    Begin(SS_OUTLINE_POLYGON);
    for (int i=0; i<len*2; i+=2)
        AppendVector(vex[i], vex[i+1]);
    End();
}

SS_VectorFrame::SS_VectorFrame(char *vectorFile)
{
    Init();
}

SS_VectorFrame::SS_VectorFrame()
{
    Init();
}

SS_VectorFrame::~SS_VectorFrame()
{
    if (gl_list) {
        glDeleteLists(gl_list, 1);
        gl_list = 0;
    }
}

//
// Init
//
void SS_VectorFrame::Init()
{
    begunUnit   = 0;
    gl_texture  = 0;
    gl_list     = 0;
    antialias   = false;
    lineWeight  = 1.0;
    width       = 0;
    height      = 0;
    xhandle     = 0.0;
    yhandle     = 0.0;
    useTint     = true;
    SetLineTint(SS_WHITE_B);
    SetFillTint(SS_WHITE_B);
}

//
// Begin
//
void SS_VectorFrame::Begin(SSVectorType m, bool front)
{
    if (begunUnit)
        throw "Nested call to SS_VectorFrame::Begin(mode)";

    //
    // Always begin a new unit for these geometries:
    //
    //  SS_TRIANGLE_FAN     SS_POLYGON      SS_OUTLINE_POLYGON
    //  SS_QUAD_STRIP       SS_LINE_LOOP
    //
    bool always = (m == SS_TRIANGLE_FAN || m == SS_POLYGON || m == SS_OUTLINE_POLYGON || m == SS_QUAD_STRIP || m == SS_LINE_LOOP);

    //
    // Only create a new unit if:
    //
    //  - Forced by the geometry type
    //  - or, The unit list is empty
    //  - or, The requested mode is a new one
    //
    if (always || unitList.Size() == 0 || unitList[front ? 0 : unitList.Size() - 1].Mode() != m)
    {
        SS_VectorUnit *newUnit = new SS_VectorUnit(m, lineWeight, useTint, lineTint, fillTint);
        front ? unitList.Prepend(newUnit) : unitList.Append(newUnit);
    }

    //
    // A unit is always activated: the first or last
    //
    begunUnit = front ? 1 : unitList.Size();
}

//
// End
//
void SS_VectorFrame::End()
{
    if (begunUnit) {
        begunUnit = 0;
        InitDisplayList();
        CalculateSize();
    }
    else
        throw "Call to SS_VectorFrame::End() without Begin(mode)!";
}

//
// PrependVector
//
void SS_VectorFrame::PrependVector(float x, float y)
{
    if (Unit())
    {
        ssVector vec = { x, y };
        Unit()->vectorList.Prepend(vec);
    }
    else
        throw "Adding vectors without Begin.";
}

//
// AppendVector
//
void SS_VectorFrame::AppendVector(float x, float y)
{
    if (Unit())
    {
        ssVector vec = { x, y };
        Unit()->vectorList.Append(vec);
    }
    else
        throw "Adding vectors without Begin.";
}

//
// PrependVectorList
//
void SS_VectorFrame::PrependVectorList(SSVectorType mode, ssVector *list, Uint16 size)
{
    Begin(mode, true);
    Unit()->vectorList.Prepend(list, size);
    End();
}

//
// AppendVectorList
//
void SS_VectorFrame::AppendVectorList(SSVectorType mode, ssVector *list, Uint16 size)
{
    Begin(mode);
    Unit()->vectorList.Append(list, size);
    End();
}

//
// PrependDuo
//
void SS_VectorFrame::PrependDuo(SSVectorType mode, float x1, float y1, float x2, float y2)
{
    ssVector vec[] = { { x1, y1 }, { x2, y2 } };
    PrependVectorList(mode, vec, 2);
}

//
// AppendDuo
//
void SS_VectorFrame::AppendDuo(SSVectorType mode, float x1, float y1, float x2, float y2)
{
    ssVector vec[] = { { x1, y1 }, { x2, y2 } };
    AppendVectorList(mode, vec, 2);
}

//
// AppendRoundedBox
//
void SS_VectorFrame::AppendRoundedBox(float x1, float y1, float x2, float y2, float r, int p, bool solid)
{
    if (p == 0) {
        p = (int)(2 * M_PI * r / 20.0f);
        if (p < 20) {
            p = (int)(2 * M_PI * r / 10.0f);
            if (p < 5) p = 5;
        }
    }

    float corner[][3] = {
            { x1 + r, y2 - r, 0.0f },
            { x1 + r, y1 + r, 90.0f },
            { x2 - r, y1 + r, 180.0f },
            { x2 - r, y2 - r, 270.0f }
        };

    if (solid)
    {
        SScolorb tempLineTint = lineTint;
        lineTint = fillTint;

        Begin(SS_TRIANGLE_FAN);

        AppendVector((x1 + x2) / 2.0f, (y1 + y2) / 2.0f);

        for (int n = 0; n <= 3; n++)
            for (int i = 0; i <= p; i++) {
                float xp = 0.0, yp = r, a = (i * 90.0) / p + corner[n][2];
                SS_Game::Rotate(xp, yp, SS_ROTINDEX(a));
                AppendVector(xp + corner[n][0], yp + corner[n][1]);
            }

        AppendVector(x1 + r, y2);   // end at bottom-left

        End();

        lineTint = tempLineTint;
    }
    else if (lineWeight > 0.0f)
    {
        SScolorb tempFillTint = fillTint;
        fillTint = lineTint;

        Begin(SS_QUAD_STRIP);

        for (int n = 0; n <= 3; n++)
            for (int i = 0; i <= p; i++) {
                float   xi = 0.0f, xo = 0.0f, yi = r, yo = r + lineWeight;
                Uint16  m = SS_ROTINDEX((i * 90.0f) / p + corner[n][2]);
                SS_Game::Rotate(xi, yi, m);
                SS_Game::Rotate(xo, yo, m);
                AppendVector(xi + corner[n][0], yi + corner[n][1]);
                AppendVector(xo + corner[n][0], yo + corner[n][1]);
            }

            AppendVector(x1 + r, y2);
            AppendVector(x1 + r, y2 + lineWeight);

        End();

        fillTint = tempFillTint;
    }

/*
    if (solid) {
        tempLineTint = lineTint;
        lineTint = fillTint;
    }

    Begin(solid ? SS_TRIANGLE_FAN : SS_LINE_LOOP);

    if (solid)
        AppendVector((x1 + x2) / 2.0f, (y1 + y2) / 2.0f);

    for (int n = 0; n <= 3; n++) {
        for (int i = 0; i <= p; i++) {
            m = (i * 90.0) / p + corner[n][2];
            AppendVector(corner[n][0] + (-r * SS_Game::Sin(SS_ROTINDEX(m))), corner[n][1] + (r * SS_Game::Cos(SS_ROTINDEX(m))));
        }
    }

    AppendVector(x1 + r, y2);

    End();

    if (solid)
        lineTint = tempLineTint;
*/
}

//
// AppendCircle
//
void SS_VectorFrame::AppendCircle(float x, float y, float r, int p, bool solid)
{
    float       m;

    if (p == 0) {
        p = (int)(r * 2 * M_PI / 20);
        if (p < 20) {
            p = (int)(r * 2 * M_PI / 10);
            if (p < 5) p = 5;
        }
    }

    Begin(solid ? SS_TRIANGLE_FAN : SS_LINE_LOOP);

    if (solid)
        AppendVector(x, y);

    for (int i = 0; i < p + (solid ? 1 : 0); i++)
    {
        m = i * 360.0 / p;
        AppendVector(x + r * SS_Game::Sin(SS_ROTINDEX(m)), y + r * SS_Game::Cos(SS_ROTINDEX(m)));
    }

    End();
}

//
// AppendRing
//  (Always solid)
//
void SS_VectorFrame::AppendRing(float x, float y, float rinner, float router, int p)
{
    if (p == 0) {
        p = (int)(rinner * 2 * M_PI / 20);
        if (p < 20) {
            p = int(router * 2 * M_PI / 10);
            if (p < 5) p = 5;
        }
    }

    Begin(SS_QUAD_STRIP);

    float m, rx, ry;
    for (int i = 0; i < p + 1; i++)
    {
        m = i * 360.0 / p;
        rx = SS_Game::Sin(SS_ROTINDEX(m));
        ry = SS_Game::Cos(SS_ROTINDEX(m));
        AppendVector(x + rinner * rx, y + rinner * ry);
        AppendVector(x + router * rx, y + router * ry);
    }

    End();
}

//
// AppendStar
//
// x, y = position
// r1   = outer radius
// r2   = inner radius
// p    = points
//
void SS_VectorFrame::AppendStar(float x, float y, float r1, float r2, int p, bool solid)
{
    float       m, r;
    float       interval = 360.0 / p / 2.0;

    if (r1 > r2) { r = r1; r1 = r2; r2 = r; }

    if (solid)
    {
        float x1, y1, x2, y2;
        for(int i=0; i < p; i++)
        {
            m = i * interval * 2;
            x1 = x + r1 * SS_Game::Sin(SS_ROTINDEX(m));
            y1 = y + r1 * SS_Game::Cos(SS_ROTINDEX(m));
            x2 = x + r1 * SS_Game::Sin(SS_ROTINDEX(m+interval*2));
            y2 = y + r1 * SS_Game::Cos(SS_ROTINDEX(m+interval*2));
            ssVector vec[] = {
                { x1, y1 },
                { x + r2 * SS_Game::Sin(SS_ROTINDEX(m+interval*1)), y + r2 * SS_Game::Cos(SS_ROTINDEX(m+interval*1)) },
                { x2, y2 },

                { x1, y1 }, { x, y }, { x2, y2 }
            };

            AppendVectorList(SS_TRIANGLES, vec, 6);
        }
    }
    else
    {
        Begin(solid ? SS_TRIANGLES : SS_LINE_LOOP);

        for(int i=0; i < p * 2; i++)
        {
            r = (i % 2) ? r2 : r1;
            m = i * interval;

            AppendVector(x + r * SS_Game::Sin(SS_ROTINDEX(m)), y + r * SS_Game::Cos(SS_ROTINDEX(m)));
        }

        End();
    }
}

//
// PopFirst
//
void SS_VectorFrame::PopFirst(Uint16 count)
{
    if (unitList.Size())
    {
        unitList.PopFirst(count);
        InitDisplayList();
    }
}

//
// PopLast
//
void SS_VectorFrame::PopLast(Uint16 count)
{
    if (unitList.Size())
    {
        unitList.PopLast(count);
        InitDisplayList();
    }
}

//
// SetLineWeight
//
void SS_VectorFrame::SetLineWeight(float w)
{
    lineWeight = w;
    if (Unit())
        Unit()->SetLineWeight(w);
}

//
// Render
//
void SS_VectorFrame::Render(float x, float y, float rot, float xscale, float yscale, const SScolorb &inTint)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(x, y, 0.0);
    glRotatef(rot, 0.0, 0.0, 1.0);
    glScalef(xscale, yscale, 0.0);

    SScolorb t = inTint;

    if (!useTint)
        MultiplyColorQuads(lineTint, t, t);

    Render(t);

    glPopMatrix();
}

//
// Render([tint])
//
//  This is a shorthand Render method which is intended to be used
//  directly by custom Render(tint) methods of SS_LayerItem subclasses.
//
//  The matrix will already have been set up by the Render(tint) method
//  by calling PushAndPrepareMatrix.
//
//  Originally created to support a sprite class that displays a vector
//  graphic when its layer's zoom gets to the point where the graphic is
//  too small. The net effect is very cool. Detailed bitmaps are rendered
//  in Close Up mode, and iconic vectors are displayed in Map View. The
//  only difference between views (from the sprite's POV) is the zoom.
//
void SS_VectorFrame::Render(const SScolorb &inTint)
{
    glColor4ubv((GLubyte*)&inTint);
    Render();
}

void SS_VectorFrame::Render()
{
    gl_do_texture(0);
    gl_do_blend(1);
    gl_antialias(antialias);
    glCallList(gl_list);
}

//
// InitDisplayList
//
void SS_VectorFrame::InitDisplayList()
{
    if (gl_list == 0)
        gl_list = glGenLists(1);

    glNewList(gl_list, GL_COMPILE);
    SendGeometry();
    glEndList();
}

//
// CalculateSize
//
void SS_VectorFrame::CalculateSize()
{
    smallx = smally = +10e20f;
    largex = largey = -10e20f;
    for (int u=0; u<unitList.Size(); u++)
    {
        SS_VectorUnit   &unit   = unitList[u];
        SS_VectorArray  &vec    = unit.vectorList;

        for (int i=0; i < vec.Size(); ++i)
        {
            float x = vec[i].x, y = vec[i].y;
            if (x < smallx) smallx = x;
            if (x > largex) largex = x;
            if (y < smally) smally = y;
            if (y > largey) largey = y;
        }
    }

    width = largex - smallx;
    height = largey - smally;
}

//
// SendGeometry
//
void SS_VectorFrame::SendGeometry(const SScolorb *inTint)
{
    if (unitList.Size() == 0)
        return;

    GLenum  line_mode, fill_mode, point_mode;
    int     i;
    bool    centerTint = false, oddEvenTint = false;

    for (int u=0; u<unitList.Size(); u++)
    {
        SS_VectorUnit   &unit   = unitList[u];
        SS_VectorArray  &vec    = unit.vectorList;
        SSVectorType    mode    = unit.Mode();
        line_mode = fill_mode = point_mode = 0;

        switch (mode)
        {

        case SS_FRAME:
        case SS_FRAME_RECT:
        case SS_RECT:
            for (i = 0; i < vec.Size(); i += 2)
            {
                if (mode != SS_FRAME)
                {
                    if (useTint)
                        glColor4ubv((GLubyte*)&unit.fillTint);
                    glRectf(vec[i].x - xhandle, vec[i].y - yhandle, vec[i+1].x - xhandle, vec[i+1].y - xhandle);
                }

                if (mode == SS_FRAME_RECT || mode == SS_FRAME)
                {
                    if (useTint)
                        glColor4ubv((GLubyte*)&unit.lineTint);

                    gl_line_width(unit.lineWeight);
                    glBegin(GL_LINE_LOOP);
                    glVertex2f(vec[i+0].x - xhandle, vec[i+0].y - yhandle);
                    glVertex2f(vec[i+1].x - xhandle, vec[i+0].y - yhandle);
                    glVertex2f(vec[i+1].x - xhandle, vec[i+1].y - yhandle);
                    glVertex2f(vec[i+0].x - xhandle, vec[i+1].y - yhandle);
                    glEnd();
                }
            }
            break;

        case SS_POINTS:
            point_mode = GL_POINTS;
            break;

        case SS_LINES:
            line_mode = GL_LINES;
            break;

        case SS_LINE_STRIP:
            line_mode = GL_LINE_STRIP;
            break;

        case SS_LINE_LOOP:
            line_mode = GL_LINE_LOOP;
            break;

        case SS_TRIANGLES:
            fill_mode = GL_TRIANGLES;
            break;

        case SS_TRIANGLE_STRIP:
            fill_mode = GL_TRIANGLE_STRIP;
            break;

        case SS_TRIANGLE_FAN:
            fill_mode = GL_TRIANGLE_FAN;
            centerTint = true;
            break;

        case SS_QUADS:
            fill_mode = GL_QUADS;
            break;

        case SS_QUAD_STRIP:
            fill_mode = GL_QUAD_STRIP;
            oddEvenTint = true;
            break;

        case SS_OUTLINE_POLYGON:
            line_mode = GL_LINE_LOOP;
        case SS_POLYGON:
            fill_mode = GL_POLYGON;
            break;
        }

        if (gl_texture)
            glBindTexture(GL_TEXTURE_2D, gl_texture);

        if (fill_mode)
        {
            if (oddEvenTint)
            {
                glBegin(fill_mode);
                for (int i=0; i<vec.Size(); i++)
                {
                    if (useTint)
                        glColor4ubv((GLubyte*)(i & 1 ? &unit.fillTint : &unit.lineTint));

                    glVertex2f(vec[i].x - xhandle, vec[i].y - yhandle);
                }
                glEnd();
            }
            else if (centerTint)
            {
                if (useTint)
                    glColor4ubv((GLubyte*)&unit.lineTint);

                glBegin(fill_mode);
                for (int i=0; i<vec.Size(); i++)
                {
                    if ( useTint && (i == 1) )
                        glColor4ubv((GLubyte*)&unit.fillTint);

                    glVertex2f(vec[i].x - xhandle, vec[i].y - yhandle);
                }
                glEnd();
            }
            else
            {
                if (useTint)
                    glColor4ubv((GLubyte*)&unit.fillTint);

                glBegin(fill_mode);
                for (int i=0; i<vec.Size(); i++)
                    glVertex2f(vec[i].x - xhandle, vec[i].y - yhandle);
                glEnd();
            }
        }

        if (line_mode)
        {
            if (useTint)
                glColor4ubv((GLubyte*)&unit.lineTint);

            gl_line_width(unit.lineWeight);
            glBegin(line_mode);
            for (int i=0; i<vec.Size(); i++)
                glVertex2f(vec[i].x - xhandle, vec[i].y - yhandle);
            glEnd();
        }

        if (point_mode)
        {
            if (useTint)
                glColor4ubv((GLubyte*)&unit.fillTint);

            glPointSize(unit.lineWeight);
            glBegin(point_mode);
            for (int i=0; i<vec.Size(); i++)
                glVertex2f(vec[i].x - xhandle, vec[i].y - yhandle);
            glEnd();
        }
    }
}

//
// Tesselate
//
//  Tesselation requires several callbacks. These are set here
//  and then each item from the unit list is tesselated.
//  As each unit tesselates the callbacks build a new unit list
//  to replace the old one.
//
void SS_VectorFrame::Tesselate()
{
    GLUtesselator   *tobj = gluNewTess();
//  gluTessCallback(tobj, GLU_TESS_VERTEX_DATA, SS_VectorFrame::vertexCallback);
//  gluTessCallback(tobj, GLU_TESS_BEGIN_DATA, SS_VectorFrame::beginCallback);
//  gluTessCallback(tobj, GLU_TESS_END_DATA, SS_VectorFrame::endCallback);
//  gluTessCallback(tobj, GLU_TESS_ERROR_DATA, SS_VectorFrame::errorCallback);
//  gluTessCallback(tobj, GLU_TESS_COMBINE_DATA, SS_VectorFrame::combineCallback);
}

void CALLBACK SS_VectorFrame::vertexCallback(GLvoid *vertex, void *inData)
{
}

void CALLBACK SS_VectorFrame::beginCallback(GLenum which, void *inData)
{
}

void CALLBACK SS_VectorFrame::endCallback(void *inData)
{
}

void CALLBACK SS_VectorFrame::errorCallback(GLenum *gl_error, void *inData)
{
}

void CALLBACK SS_VectorFrame::combineCallback(GLdouble coords[3], GLdouble *vdata[4], GLfloat weight[4], GLdouble **outData, void *inData)
{
}


#pragma mark -
//--------------------------------------------------------------
// SS_VectorSprite
//
SS_VectorSprite::SS_VectorSprite()
{
    Init();
}

SS_VectorSprite::~SS_VectorSprite()
{
}

void SS_VectorSprite::Init()
{
}

//
// AddFrame
//
// SS_VectorSprite uses the array template so this
// method copies the array size to the LayerItem.
//
void SS_VectorSprite::AddFrame(SS_VectorFrame *frame)
{
    #if SS_DEBUG
    printf("[%08X] SS_VectorSprite::AddFrame(frame) (%d)\n", this, frameCount);
    #endif

    frameArray.Append(frame);
    frame->Retain("Frame in VectorSprite");

    frameCount = frameArray.Size();

    if (frame->width > width)
        width = frame->width;

    if (frame->height > height)
        height = frame->height;
}

const SS_VectorSprite& SS_VectorSprite::operator=(const SS_VectorSprite &src)
{
    if (&src != this)
    {
        SS_LayerItem::operator=(src);

        for (int i=0; i < src.frameCount; ++i)
            AddFrame(src.frameArray[i]);
    }

    return *this;
}

//
// ReleaseFrames
//
void SS_VectorSprite::ReleaseFrames()
{
    for (int i=0; i < frameCount; ++i)
        frameArray[i]->Release();
}

//
// SetHandle(x, y)
// Set the handle position of all the sprite's frames
//
void SS_VectorSprite::SetHandle(float x, float y)
{
    #if SS_DEBUG
    printf("[%08X] SS_VectorSprite::SetHandle(%d, %d)\n", this, x, y);
    #endif

    SS_LayerItem::SetHandle(x, y);

    if (frameCount)
        for (int i=0; i < frameCount; ++i)
            frameArray[i]->SetHandle(x, y);
}


//
// CenterHandle
//
void SS_VectorSprite::CenterHandle()
{
    #if SS_DEBUG
    printf("[%08X] SS_VectorSprite::SetHandle(%d, %d)\n", this, x, y);
    #endif

    if (frameCount)
        for (int i=0; i < frameCount; ++i)
            frameArray[i]->CenterHandle();
}


//
// Render
//
void SS_VectorSprite::Render(const SScolorb &inTint)
{
    SScolorb outTint;
    MultiplyColorQuads(inTint, tint, outTint);

    SS_World    *w = World();
    float       z = w->Zoom();
    float       xs = xscale, ys = yscale;

    //
    // Reverse scaling on non-zoomed objects in a zoomable layer
    //
    if ((flags & SS_NOZOOM) && !(layer->flags & SS_NOZOOM)) {
        xs /= z;
        ys /= z;
    }

    frameArray[currFrame]->Render(xpos, ypos, rotation, xs, ys, outTint);
}


//
// IsOnScreen
// Determine if something is on the screen by comparing its
// visible position in the universe to the view port area.
//
bool SS_VectorSprite::IsOnScreen()
{
    return true;

    SS_Point    point;
    GlobalPosition(&point);

    float   rcos = SS_Game::Cos(point.i);
    float   rsin = SS_Game::Sin(point.i);

    // Get the sides of the sprite
    SS_VectorFrame  *fr = frameArray[currFrame];
    float   le = -fr->xhandle * xscale;
    float   ri = le + fr->width * xscale;

    if ((ri - le) > SS_VIDEO_W)
        return true;

    float   to = -fr->yhandle * yscale;
    float   bo = to + fr->height * yscale;

//  printf("(%.2f, %.2f) - (%.2f, %.2f)\n", le, to, ri, bo);

    SS_World    *w = World();

    // If nozoom adjust sides to compensate
    if ((flags|layer->flags) & SS_NOZOOM) {
        float z = w->Zoom();
        le /= z; to /= z;
        ri /= z; bo /= z;
    }

    // Get the extents of the rotated box
    float   left, right, top, bottom;

    if (rcos > 0)
    {
        if (rsin > 0) { // SE
            left = (le * rcos - bo * rsin);     // b-l
            right = (ri * rcos - to * rsin);    // t-r
            top = (to * rcos + le * rsin);      // t-l
            bottom = (bo * rcos + ri * rsin);   // b-r
        } else {        // NE
            left = (le * rcos - to * rsin);     // t-l
            right = (ri * rcos - bo * rsin);    // b-r
            top = (to * rcos + ri * rsin);      // t-r
            bottom = (bo * rcos + le * rsin);   // b-l
        }
    } else {
        if (rsin > 0) { // SW
            left = (ri * rcos - bo * rsin);     // b-r
            right = (le * rcos - to * rsin);    // t-l
            top = (bo * rcos + le * rsin);      // b-l
            bottom = (to * rcos + ri * rsin);   // t-r
        } else {        // NW
            left = (ri * rcos - to * rsin);     // t-r
            right = (le * rcos - bo * rsin);    // b-l
            top = (bo * rcos + ri * rsin);      // b-r
            bottom = (to * rcos + le * rsin);   // t-l
        }
    }

    // Get the screen-local coordinates (0,0 is top-left)
    float   x = point.x - w->left;
    float   y = point.y - w->top;

    // Zoomed width and height
    float   wi = w->ZoomWidth();
    float   hi = w->ZoomHeight();

    // Test for any overlap with screen
    return !(x + left > wi || x + right < 0 || y + top > hi || y + bottom < 0);
}


//
// TestPointCollision
//
bool SS_VectorSprite::TestPointCollision(float x, float y, bool isLocal)
{
    SS_Point    apoint;
    Uint16      irot;
    Uint8       *thisMask;
    float       usin, ucos;
    float       rx, ry;

    SS_VectorFrame  *thisFrame = frameArray[currFrame];
    Uint16      tw = thisFrame->width;
    Uint16      th = thisFrame->height;

    if (isLocal)
        LayerPosition(&apoint);
    else
        GlobalPosition(&apoint);

    x -= apoint.x;  y -= apoint.y;

    // - unrotate and untranslate the point, then make corner-relative
    irot = -apoint.i;
    ucos = SS_Game::Cos(irot);
    usin = SS_Game::Sin(irot);
    rx = (x * ucos - y * usin) + thisFrame->xhandle - thisFrame->smallx;
    ry = (y * ucos + x * usin) + thisFrame->yhandle - thisFrame->smally;

    // - is the point out of bounds?
    if (rx < 0 || rx >= tw || ry < 0 || ry >= th)
        return false;

    return true;
}
