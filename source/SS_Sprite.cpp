/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Sprite.cpp
 *
 *  $Id: SS_Sprite.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Sprite.h"

#include "SS_Game.h"
#include "SS_SFont.h"
#include "SS_Layer.h"
#include "SS_Types.h"
#include "SS_Utilities.h"

#include <stdlib.h>

//--------------------------------------------------------------
// SS_Sprite
// A graphical object with one or more frames
//--------------------------------------------------------------

SS_Sprite::SS_Sprite()
{
    DEBUGF(1, "[%08X] SS_Sprite() CONSTRUCTOR\n", this);

    Init();
}

//
// SS_Sprite(filename) constructor
//
SS_Sprite::SS_Sprite(char *spriteFile)
{
    Init();
}

SS_Sprite::~SS_Sprite()
{
    DEBUGF(1, "[%08X] ~SS_Sprite()\n", this);

    ReleaseFrames();
}


//
// operator=
//
const SS_Sprite& SS_Sprite::operator=(const SS_Sprite &src)
{
    DEBUGF(1, "[%08X] SS_Sprite::operator=(%08X)\n", this, &src);

    if (&src != this) {
        SS_Collider::operator=(src);
        ReleaseFrames();
        for (int i=0; i < src.FrameCount(); i++)
            AddFrame(src.frameArray[i]);
    }

    return *this;
}


//
// Init()
// Initialize a sprite
//
void SS_Sprite::Init()
{
    DEBUGF(1, "[%08X] SS_Sprite::Init()\n", this);

    frameArray  = NULL;
    frameBlocks = 0;
}


//
// ReleaseFrames
// Remove all frames from the sprite
//
void SS_Sprite::ReleaseFrames()
{
    DEBUGF(1, "[%08X] SS_Sprite::ReleaseFrames()\n", this);

    if (frameCount > 0)
    {
        for (int i=0; i < frameCount; i++)
            (void)frameArray[i]->Release();

        frameCount = 0;
        frameBlocks = 0;
        free(frameArray);
    }
}


#pragma mark -
//
// SetWorld(world)
//
void SS_Sprite::SetWorld(SS_World *w)
{
    DEBUGF(1, "[%08X] SS_Sprite::SetWorld(%08X)\n", this, w);

    if ((world = w))
    {
        oldW = w->ZoomWidth();
        oldH = w->ZoomHeight();

        if (!isCollider && (collisionIn || collisionOut))
            AddToColliders();
    }
    else if (isCollider)
        RemoveFromColliders();
}


//
// AddFrame(framePtr)
// Add a frame to the sprite
//
void SS_Sprite::AddFrame(SS_Frame *frame)
{
    DEBUGF(1, "[%08X] SS_Sprite::AddFrame(%d)\n", this, frameCount);

    if ((frameCount % SS_FRAME_BLOCK) == 0)
    {
        frameBlocks++;

        if (!frameArray)
            frameArray = (SS_Frame**)malloc(sizeof(SS_Frame*) * SS_FRAME_BLOCK);
        else
            frameArray = (SS_Frame**)realloc(frameArray, sizeof(SS_Frame*) * SS_FRAME_BLOCK * frameBlocks);

        if (!frameArray)
            throw "Can't allocate memory for Frames.";
    }

    frameArray[frameCount++] = frame;
    frame->Retain("Frame in Sprite");

    if (frame->width > width)
        width = frame->width;

    if (frame->height > height)
        height = frame->height;
}

//
// SetAnimRange(start, end)
//
void SS_Sprite::SetAnimRange(Uint16 start, Uint16 end)
{
    if (start >= frameCount) start = frameCount - 1;
    if (end >= frameCount) end = frameCount - 1;
    SS_Collider::SetAnimRange(start, end);
}

//
// SetFrameIndex(f)
//
// Set the frame index and update the sprite's handle
// based on the handle of the frame.
//
void SS_Sprite::SetFrameIndex(Uint16 f)
{
    SS_Collider::SetFrameIndex(f);
    frameArray[currFrame]->GetHandle(&xhandle, &yhandle);
}

//
// SetHandle(frameIndex, x, y)
// Set the handle position of a single frame
//
void SS_Sprite::SetHandle(Uint16 frame, float x, float y)
{
    if (frame < frameCount)
        frameArray[frame]->SetHandle(x, y);
}

//
// SetHandle(x, y)
// Set the handle position of all the sprite's frames
//
void SS_Sprite::SetHandle(float x, float y)
{
    SS_Collider::SetHandle(x, y);

    for (int i=0; i<frameCount; i++)
        SetHandle(i, x, y);
}

//
// CenterHandle
// Center the handle of all the frames of the sprite
//
void SS_Sprite::CenterHandle()
{
    SS_Collider::CenterHandle();

    if (frameCount)
        for (int i=0; i<frameCount; i++)
            CenterHandle(i);
}

//
// CenterHandle(frameIndex)
// Center the handle of a single frame of the sprite
//
void SS_Sprite::CenterHandle(Uint16 frame)
{
    if (frame < frameCount)
        frameArray[frame]->CenterHandle();
}

//
// Render
//
void SS_Sprite::Render(const SScolorb &inTint)
{
    if ( IsVisible() )
    {
        float       x = xpos, y = ypos;
        float       xs = xscale, ys = yscale;
        SS_World    *w = World();
        float       z = w->Zoom();
        Uint32      f = flags;

        //
        // Reverse scaling on non-zoomed objects in a zoomable layer
        //
        if ((f & SS_NOZOOM) && !(layer->flags & SS_NOZOOM)) {
            xs /= z;
            ys /= z;
        }

        //
        // Absolute items' coordinates are relative to the view
        //
        if (f & SS_NOSCROLL)
        {
            x += w->left;
            y += w->top;

            if (!(f & SS_NOZOOM) && !(layer->flags & SS_NOSCROLL)) {
                x += (w->ZoomWidth() - oldW) / 2;
                y += (w->ZoomHeight() - oldH) / 2;
            }
        }
        else
        {
            // A Spatially scaled layer?
            float   s = layer->spatialScale;
            if (s != 1.0)
            {
                float wl = w->left, wt = w->top;

                x += wl - (wl / s);
                y += wt - (wt / s);
            }
        }

        SScolorb outTint;
        MultiplyColorQuads(inTint, tint, outTint);
        frameArray[currFrame]->Render(x, y, rotation, xs, ys, outTint);
    }
}


//
// IsOnScreen
// Determine if something is on the screen by comparing its
// visible position in the universe to the view port area.
//
bool SS_Sprite::IsOnScreen()
{
    SS_Point    point;
    GlobalPosition(&point);

    float   rcos = SS_Game::Cos(point.i);
    float   rsin = SS_Game::Sin(point.i);

    // Get the sides of the sprite
    SS_Frame    *fr = frameArray[currFrame];
    float   le = -fr->xhandle * xscale;
    float   to = -fr->yhandle * yscale;
    float   ri = le + fr->width * xscale;
    float   bo = to + fr->height * yscale;

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

/*
    if (group)
        printf("L:%.1f T:%.1f R:%.1f B:%.1f\n", x+left, y+top, x+right, y+bottom);
*/

    // Test for any overlap with screen
    return !(x + left > wi || x + right < 0 || y + top > hi || y + bottom < 0);
}


#pragma mark -
//
// _TestCollision(item)
//
bool SS_Sprite::_TestCollision(SS_Collider *otherItem)
{
    SS_Sprite   *smaller, *large, *other = (SS_Sprite*)otherItem;
    SS_Point    spoint, lpoint;
    SS_Frame    *smallFrame, *largeFrame;
    Uint16      x, y, smaskw, lmaskw, sw, sh, lw, lh;
    float       ssin, scos, usin, ucos;
    float       scx, scy, shx, shy, rx, ry, testx, testy;
    float       small_x, small_y, large_x, large_y;
    Sint16      intx, inty;
    Uint16      irot;

    SS_Frame    *thisFrame = frameArray[currFrame];
    SS_Frame    *otherFrame = other->frameArray[other->currFrame];

    // 1. Get the width and height of both sprites
    Uint16      tw = static_cast<Uint16>(thisFrame->width);
    Uint16      th = static_cast<Uint16>(thisFrame->height);
    Uint16      ow = static_cast<Uint16>(otherFrame->width);
    Uint16      oh = static_cast<Uint16>(otherFrame->height);

/*
    float       tscale = 1.0;

    Get the scaling factors...

    txs = xscale;
    tys = yscale;
    oxs = other->xscale;
    oys = other->yscale;

    bool        testMask = true;
    bool        otherIsLarger;
    if (txs != tys || oxs != oys) {
        testMask = false;
        otherIsLarger = true;
    }
    else if (txs != 1 || oxs != 1)
    {
        otherIsLarger = (txs < oxs);
        if (otherIsLarger)
            tscale = oxs;
        else
            tscale = txs;

        tw /= tscale;
        th /= tscale;
        ow /= tscale;
        oh /= tscale;
    }
    else
        otherIsLarger = (tw * th < ow * oh);

    Get the scaled widths and heights

    stw = tw * txs;
    sth = th * tys;
    sow = ow * oxs;
    soh = oh * oys;
*/

    // 2. Determine which frame is larger
    if (tw * th < ow * oh) {
        large = other;
        largeFrame = otherFrame;
        lw = ow; lh = oh;

        smaller = this;
        smallFrame = thisFrame;
        sw = tw; sh = th;
    } else {
        large = this;
        largeFrame = thisFrame;
        lw = tw; lh = th;

        smaller = other;
        smallFrame = otherFrame;
        sw = ow; sh = oh;
    }

    large->GlobalPosition(&lpoint);
    large_x = lpoint.x;
    large_y = lpoint.y;

    smaller->GlobalPosition(&spoint);
    small_x = spoint.x;
    small_y = spoint.y;


    // 3. Normalize based on the larger sprite

    // - the un-rotation of the larger
    irot = -lpoint.i;
    ucos = SS_Game::Cos(irot);
    usin = SS_Game::Sin(irot);

    // - the adjusted handle of the smaller
    shx = small_x - large_x;
    shy = small_y - large_y;
    rx = (shx * ucos - shy * usin);
    ry = (shy * ucos + shx * usin);
    shx = rx;
    shy = ry;

    // - the adjusted rotation of the smaller
    irot += spoint.i;
    scos = SS_Game::Cos(irot);
    ssin = SS_Game::Sin(irot);

    // - rotate the smaller's corner
    scx = -smallFrame->xhandle;
    scy = -smallFrame->yhandle;
    rx = (scx * scos - scy * ssin);
    ry = (scy * scos + scx * ssin);

    // - small's corner relative to large's corner
    scx = shx + rx + largeFrame->xhandle;
    scy = shy + ry + largeFrame->yhandle;


    // 4. Test the rough intersection of the two sprites

#define xover   (sw * scos)
#define xdown   (sh * -ssin)
#define yover   (sw * ssin)
#define ydown   (sh * scos)
#define LEFT    0
#define RIGHT   lw
#define TOP     0
#define BOTTOM  lh

    if (scos > 0) {
        if (ssin > 0) {     // SE
            if ((scy > BOTTOM) || (scx + xover < LEFT) || (scx + xdown > RIGHT) || (scy + yover + ydown < TOP))
                return false;
        } else {            // NE
            if ((scx > RIGHT) || (scy + ydown < TOP) || (scy + yover > BOTTOM) || (scx + xover + xdown < LEFT))
                return false;
        }
    } else {
        if (ssin > 0) {     // SW
            if ((scx < LEFT) || (scy + yover < TOP) || (scy + ydown > BOTTOM) || (scx + xover + xdown > RIGHT))
                return false;
        } else {            // NW
            if ((scy < TOP) || (scx + xdown < LEFT) || (scx + xover > RIGHT) || (scy + yover + ydown > BOTTOM))
                return false;
        }
    }


    // sprites lacking masks only roughly collide
    Uint8 *smallmask, *largemask;
    if (!(largemask = largeFrame->mask) || !(smallmask = smallFrame->mask))
        return true;


    // 5. Iterate through the small sprite
    //  testing for collisions as we go

    lmaskw = (lw + 7) / 8;
    smaskw = (sw + 7) / 8;
    for (y = 0; y < sh; y++)
    {
        testy = scy;
        testx = scx;
        for (x = 0; x < sw; x++)
        {
            inty = int(testy + 0.5);
            intx = int(testx + 0.5);

            if (
                (inty >= 0 && inty < lh && intx >= 0 && intx < lw)
                && ( smallmask[(x / 8) + (y * smaskw)] & (1 << (x % 8)) )
                && ( largemask[(intx / 8) + (inty * lmaskw)] & (1 << (intx % 8)) )
                )
            {
                return true;
            }

            testx += scos;
            testy += ssin;
        }

        scx -= ssin;
        scy += scos;
    }

    return false;
}


//
// TestPointCollision(x, y)
//
bool SS_Sprite::TestPointCollision(float x, float y, bool isLocal)
{
    SS_Point    apoint;
    Uint16      irot;
    Uint8       *thisMask;
    float       usin, ucos;
    float       rx, ry;

    SS_Frame    *thisFrame = frameArray[currFrame];
    float       tw = thisFrame->width;
    float       th = thisFrame->height;

    if (isLocal)
        LayerPosition(&apoint);
    else
        GlobalPosition(&apoint);

    x -= apoint.x;  y -= apoint.y;

    // - unrotate and untranslate the point, then make corner-relative
    irot = -apoint.i;
    ucos = SS_Game::Cos(irot);
    usin = SS_Game::Sin(irot);
    rx = (x * ucos - y * usin) + thisFrame->xhandle;
    ry = (y * ucos + x * usin) + thisFrame->yhandle;

    // - is the point out of bounds?
    if (rx < 0 || rx >= tw || ry < 0 || ry >= th)
        return false;

    // - is there a mask?
    if (!(thisMask = thisFrame->mask))
        return true;

    // - test the local point in the mask
    Sint16  testx = int(rx + 0.5);
    Sint16  testy = int(ry + 0.5);

    if ( thisMask[(testx / 8) + (testy * thisFrame->maskBytesWide)] & (1 << (testx % 8)) )
        return true;

    return false;
}


