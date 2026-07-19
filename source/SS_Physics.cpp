/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Physics.cpp
 *
 *  $Id$
 *
 */

#include "SS_Physics.h"

#if SS_PHYSICS_ENABLE

//--------------------------------------------------------------
// SS_Physics
// Wraps a b2WorldId physics simulation (Box2D 3.x C API).
//--------------------------------------------------------------

SS_Physics::SS_Physics(b2Vec2 gravity)
    : mWorldId(b2_nullWorldId)
    , mSubStepCount(4)
{
    b2WorldDef def = b2DefaultWorldDef();
    def.gravity = gravity;
    mWorldId = b2CreateWorld(&def);
}

SS_Physics::~SS_Physics()
{
    if (b2World_IsValid(mWorldId))
        b2DestroyWorld(mWorldId);
    mWorldId = b2_nullWorldId;
}

//
// Step(dt, subStepCount)
// Advance the physics simulation.
//
void SS_Physics::Step(float dt, int subStepCount)
{
    if (b2World_IsValid(mWorldId))
        b2World_Step(mWorldId, dt, subStepCount);
}

//
// SetGravity(x, y)
// Set world gravity in m/s².
//
void SS_Physics::SetGravity(float x, float y)
{
    if (b2World_IsValid(mWorldId))
        b2World_SetGravity(mWorldId, b2Vec2{x, y});
}

#endif // SS_PHYSICS_ENABLE
