/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Physics.h
 *
 *  Box2D physics subsystem — optional, gated behind SS_PHYSICS_ENABLE.
 *
 *  Coordinate convention:
 *    PIXELS_PER_METER (PPM) = 32.0f
 *    Box2D runs in meters; SimpleSprite runs in pixels.
 *    All conversions happen at the SS_Physics / SS_LayerItem boundary.
 *
 *  $Id$
 *
 */

#ifndef __SS_PHYSICS_H__
#define __SS_PHYSICS_H__

#include "SS_Config.h"

#if SS_PHYSICS_ENABLE

#include <box2d/box2d.h>

// ---------------------------------------------------------------------------
// Pixels-per-meter scale factor. Box2D operates in metres internally;
// SimpleSprite positions/rotations are in pixels. Conversion happens in
// SS_LayerItem::SyncPhysicsBody() and when creating bodies.
// ---------------------------------------------------------------------------
constexpr float PPM = 32.0f;        // pixels per meter

// ---------------------------------------------------------------------------
// SS_Physics
//
// Owns a b2WorldId physics simulation (Box2D 3.x C API). Call Step() from
// the world's Process() loop to advance the simulation. Per-item transform
// sync is handled by SS_World / SS_LayerItem.
// ---------------------------------------------------------------------------
class SS_Physics
{
    private:
        b2WorldId       mWorldId;
        int             mSubStepCount;      // sub-steps per call to Step

    public:
                        SS_Physics(b2Vec2 gravity = b2Vec2{0.0f, -10.0f});
        virtual         ~SS_Physics();

        // Access the underlying Box2D world id (publicly accessible for
        // direct Box2D API calls from game code).
        inline b2WorldId WorldId() const            { return mWorldId; }

        // Step the physics simulation forward by dt seconds.
        void            Step(float dt, int subStepCount = 4);

        // Set gravity in m/s².
        void            SetGravity(float x, float y);

        // Sub-step count accessors
        inline int      SubStepCount() const        { return mSubStepCount; }
        inline void     SetSubStepCount(int n)      { mSubStepCount = n; }

        // Validity check
        inline bool     IsValid() const             { return b2World_IsValid(mWorldId); }
};

#endif // SS_PHYSICS_ENABLE
#endif // __SS_PHYSICS_H__
