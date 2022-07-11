/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Collisions.cpp
 *
 *  $Id: SS_Collisions.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Collisions.h"

#include "SS_Game.h"
#include "SS_ItemGroup.h"
#include "SS_World.h"

//--------------------------------------------------------------
//
//  COLLISION_SLEW indicates the fraction of the lists which
//  will be collision-tested per frame. Larger slew values may
//  increase frame rates slightly but also will reduce the
//  frequency of tests. Fine tune for your own situation.
//
//  This is one means of making collisions go faster. See
//  optimization notes below for upcoming development paths.
//
#define COLLISION_SLEW  2

//
// SLEW_SEGMENT_SIZE calculates lists divided by slew
//
#define SLEW_SEGMENT_SIZE       (SS_COLLISION_LISTS/COLLISION_SLEW)

//
//--------------------------------------------------------------

#define SPATIAL_INDEX(x)    ( ((x) < 0) \
                            ? (SS_COLLISION_LISTS - 1) - (Uint16)floorf(-(x) / SS_COLLISION_BAND_SIZE) % SS_COLLISION_LISTS \
                            : (Uint16)floorf((x) / SS_COLLISION_BAND_SIZE) % SS_COLLISION_LISTS )


//--------------------------------------------------------------
//
// SS_CollisionManager
//
//  Manages colliders in an optimal manner, dividing them up into
//  linked lists based on collision id and physical position.
//
//  This is implemented as linked lists so the nodes may be
//  scattered and non-optimal for caching, so reducing tests is
//  important. Lists are likely to change often, but may still
//  be optimizable by implementing using self-sizing arrays.
//
//--------------------------------------------------------------


SS_CollisionManager::SS_CollisionManager()
{
    Init();
}

void SS_CollisionManager::Init()
{
/*
    for (int j=0; j < SS_COLLISION_TYPES; i++)
        for (int i=0; i < SS_COLLISION_LISTS; i++)
            colliderList[j][i].Clear();
*/

    for (int i=0; i < SS_COLLISION_LISTS; i++)
        colliderList[i].Clear();
}

//
//  Collision lists represent swaths of space of a given
//  width and infinite height. So if you use 200 lists
//  with each representing a 512 pixel-wide swath, the
//  list encompasses all colliders in each 102,400 pixel-
//  wide region of the world in a single model. Every
//  item in each list will either be very close to each
//  other or very far away. In this arrangement it is only
//  necessary to test collisions between an item and the
//  items which follow it in its own list, and only its
//  right neighboring collision list.
//
//  The side-effect of this scheme is that the collision
//  lists need to be dynamic. Items need to be moved to the
//  appropriate collision list in response to motion in the
//  world. (TODO: Allow custom rules for placement?)
//

//
// AddToColliders
// Add a layeritem to the collision pool
//

SS_ColliderNode* SS_CollisionManager::AddToColliders(SS_Collider *item)
{
    if (item->Layer() && item->World())
    {
        SS_Point    pos;
        item->GlobalPosition(&pos);
        float xx = pos.x;
        return colliderList[SPATIAL_INDEX(xx)].Append(item);
    }
    else
        return NULL;
}

//
// SS_World::RunCollisionTest
// Test all the sprite collisions in the world
//

//
// Optimization Notes (TODO:)
//
//  The current model is a single self-migrating multi-list
//  that moves nodes from list to list based on their spatial
//  (x) position.
//  Node propagation is clean and efficient as possible -
//  implemented at the linked list template class level.
//  However, it is currently implemented by SS_LayerItem
//  rather than SS_CollisionManager
//
//  Node propagation won't need to change much, except to
//  move node propagation to an encapsulating class.
//
//  The first optimization will be to further break these
//  collision lists into discrete groups based on the
//  collision id (collisionOut) of their members - a single
//  number.
//
//  RunCollisionTest will then only test each item against
//  the collision lists of the items which can collide
//  with it, and which are also in its spatial sphere.
//
//  Aa with the spatial lists, colliders don't need to check
//  against lower-numbered lists.
//
//  An optimal scheme would maintain a Generate / Receive
//  list, with all unique pairs represented.
//
//
//  layerItem->SetCollisionOut(COLL_SHIP);  // add this item to the COLL_SHIP collision-group (and remember the node)
//  layerItem->SetCollisionIn(COLL_ROCK);   // append COLL_ROCK to its node's list of groups to test
//  layerItem->ClearCollisionIn(COLL_ROCK); // remove COLL_ROCK from its node's list of groups to test
//  layerItem->ClearCollisionOut();         // remove this item from its collision-group (and forget the node)
//
//
void SS_CollisionManager::RunCollisionTest()
{
    #if COLLISION_SLEW > 1
        static  Uint16 collisionSlew = 0;
    #else
        const   Uint16 collisionSlew = 0;
    #endif

/*
    static Uint32   collTick;
    if (GetWorldTime() - collTick < 5)
        return;
    collTick = GetWorldTime();
*/

    #if COLLISION_SLEW > 1
    collisionSlew = ++collisionSlew % COLLISION_SLEW;
    #endif

    SS_Collider         *item, *inner;
    SS_ColliderIterator outer_iter, inner_iter;

    //
    // Test a group of lists, or all if SLEW == 1
    //
    for ( unsigned i = collisionSlew * SLEW_SEGMENT_SIZE; i < (collisionSlew + 1) * SLEW_SEGMENT_SIZE; i++ )
    {
        if ( colliderList[i].m_count > 0 )
        {
            outer_iter = colliderList[i].GetIterator();

            while ((item = outer_iter.NextItem()))
            {
                if ( item->IsVisible() )
                {
                    // Test all items following in the same list
                    inner_iter = outer_iter;
                    while ((inner = inner_iter.NextItem()))
                        if ( item->TestCollision(inner) )
                            item->CollideWith(inner), inner->CollideWith(item);

                    // Test all items in the list to the right
                    inner_iter = colliderList[(i + 1) % SS_COLLISION_LISTS].GetIterator();
                    while ((inner = inner_iter.NextItem()))
                        if ( item->TestCollision(inner) )
                            item->CollideWith(inner), inner->CollideWith(item);
                }
            }
        }
    }
}


//
// CollidersAtPoint
// Find the first item that lies on the line
//
SS_ColliderList* SS_CollisionManager::CollidersAtPoint(float x, float y)
{
    SS_ColliderList *list = new SS_ColliderList();
    return list;
}

SS_Collider* SS_CollisionManager::FirstColliderOnLine(float x1, float y1, float x2, float y2)
{
    return NULL;
}

SS_Collider* SS_CollisionManager::FirstColliderAt(float x, float y)
{
    SS_Collider         *item;
    SS_ColliderIterator iter;

    // Test the three lists neighboring this point
    unsigned xlist = SPATIAL_INDEX(x);
    for ( unsigned i=0; i<=2; i++ )
    {
        unsigned curr = (xlist + i + SS_COLLISION_LISTS - 1) % SS_COLLISION_LISTS;
        if ( colliderList[curr].m_count )
        {
            iter = colliderList[curr].GetIterator();
            while ((item = iter.NextItem()))
                if ( item->IsVisible() && item->TestPointCollision(x, y) )
                    return item;
        }
    }

    return NULL;
}

void SS_CollisionManager::DrawCollisionGraph()
{
//  glMatrixMode(GL_PROJECTION);
//  glLoadIdentity();
//  glOrtho(0, SS_VIDEO_W, SS_VIDEO_H, 0, -1.0f, 1.0f);

    //
    // Draw a bar graph of the collision list lengths
    //
    glColor4ub(0xF0, 0xFF, 0x00, 0xFF);
    for (int q=0; q < SS_COLLISION_LISTS; q++)
    {
        float x1 = 10 + q * 10;
        float x2 = x1 + 8;
        float y2 = SS_VIDEO_H - 5;

        for (int j=0; j<colliderList[q].m_count; j++)
        {
            float y1 = SS_VIDEO_H - 5 - j * 5;
            glRectf(x1, y1, x2, y1 + 3);
        }
    }
}


#pragma mark -
//--------------------------------------------------------------
//
//  SS_Collider     Layer item collision support class
//
//--------------------------------------------------------------

//
// Init
//
void SS_Collider::Init()
{
    collNode            = NULL;
    isCollider          = false;
    collisions          = 0;
    collisionOut        = 0;
    collisionIn         = 0;
    collisionUpdated    = false;
    collisionIgnore     = 0;
    collisionSource     = 0;
    listNumber          = -1;
}

//
// SetWorld(world)
//  Set the world for this item. Currently the
//  collider list is encapsulated in SS_World so here
//  it also adds or removes the item as necessary
//  from the appropriate collision list.
//
void SS_Collider::SetWorld(SS_World *w)
{
    DEBUGF(1, "[%08X] SS_Collider::SetWorld(%08X)\n", this, w);

    SS_LayerItem::SetWorld(w);

    if (w)
    {
        if ( collisionOut || collisionIn )
            AddToColliders();
    }
    else if (isCollider)
        RemoveFromColliders();
}

//
// RemoveSelf
// Unlink this item from its group's or layer's linked list
//
void SS_Collider::RemoveSelf()
{
    DEBUGF(1, "[%08X] SS_Collider::RemoveSelf()\n", this);

    RemoveFromColliders();

    SS_LayerItem::RemoveSelf();
}


//
// _Process
//
//  Do all necessary processing based on time elapsed
//  and update its node in the collision lists.
//
void SS_Collider::_Process()
{
    SS_LayerItem::_Process();
    UpdateNodePosition();
}

//
// operator=(other)
//  Copy another collider to this instance.
//
//  This is very much presumed to be used in
//  copy construction but not as a means to
//  make an existing layer item become a perfect
//  duplicate of another active layer item (although
//  that would be ideal).
//
const SS_Collider& SS_Collider::operator=(const SS_Collider &src)
{
    DEBUGF(1, "[%08X] SS_LayerItem::operator=(%08X)\n", this, &src);

    if (&src != this)
    {
        SS_LayerItem::operator=(src);

        // Initialize collision membership
        isCollider      = false;
        collNode        = NULL;
        listNumber      = -1;

        // Initialize collision testing
        collisions      = 0;
        collisionIgnore = 0;
        collisionUpdated= false;

        // Some are copied directly
        collisionOut    = src.collisionOut;
        collisionIn     = src.collisionIn;
        collisionSource = src.collisionSource;
    }

    return *this;
}

//
// EnableCollisions(out, in)
// Set which collisions to accept and generate
//
void SS_Collider::EnableCollisions(Uint32 out, Uint32 in)
{
    DEBUGF(1, "[%08X] SS_Collider::EnableCollisions(%04X, %04X)\n", this, out, in);

    collisionOut = out;
    collisionIn = in;
    collisionIgnore = 0;

    if (world)
    {
        if (out || in)
        {
            if (!isCollider)
                AddToColliders();
        }
        else if (isCollider)
            RemoveFromColliders();
    }
}

//
// AddToColliders
// Add the sprite to the collision pool
//
void SS_Collider::AddToColliders()
{
    if (!isCollider && layer && world) {
        collNode = world->AddToColliders(this);
        isCollider = true;
    }
}

//
// RemoveFromColliders
// Remove the sprite from the collision pool
//
void SS_Collider::RemoveFromColliders()
{
    if (isCollider) {
        ColliderNode()->RemoveSelf();
        isCollider = false;
        collNode = NULL;
    }
}

//
// UpdateNodePosition
//
void SS_Collider::UpdateNodePosition()
{
    if (isCollider)
    {
        Uint16 x = GetSpatialIndex();
        if (listNumber != x) {
            collNode->Migrate(&world->colliderList[x]);
            listNumber = x;
        }
    }
}

//
// GetSpatialIndex
//
//  Colliders calculate their own list index.
//  This is so the collision manager doesn't have
//  to pull out the item's entrails to get at this
//  information.
//  This still relies on global values and is
//  presumed not to be overridden too often.
//
Uint16 SS_Collider::GetSpatialIndex()
{
    SS_Point pos;
    GlobalPosition(&pos);
    float xx = pos.x;
    return SPATIAL_INDEX(xx);
}

SS_ColliderList* SS_Collider::ColliderList()
{
    return collNode->m_container;
}

/*
SS_ColliderList** SS_Collider::ColliderListList()
{
}
*/

//
// UpdateCollisions
// Test this sprite's collisions against all nearby colliders
//
void SS_Collider::UpdateCollisions()
{
    if ( IsVisible() )
    {
        SS_World    *w = World();
        SS_Collider   *item;

        int b = listNumber + SS_COLLISION_LISTS - 1;
        for (int i=0; i<=2; i++)
        {
            SS_ColliderIterator itr = w->colliderList[(b+i) % SS_COLLISION_LISTS].GetIterator();
            while ((item = itr.NextItem()))
                if (item != this && TestCollision(item))
                    CollideWith(item);
        }
    }
}

//
// CollideWith
//
// Receive a Collision from another item. Default response is to
// Add to the collision mask and send the collision to the container
// object, if any. Responding at once saves time later on.
//
void SS_Collider::CollideWith(SS_Collider *s)
{
    CollisionAdd(s);
    if (group) group->CollideWith(s);
}

//
// GetNewCollisions
// Get new collisions since the last time they were updated
//
Uint32 SS_Collider::GetNewCollisions()
{
    Uint32  coll;

    if (collisionUpdated)
    {
        collisionUpdated = false;

        // 1. Ignore collisions that were already set

        if ((coll = collisions))            // get actual collisions
        {
            coll &= ~collisionIgnore;       // remove ignored collisions
            collisionIgnore |= coll;        // add the rest to the ignore list
        }
        else
            collisionIgnore = 0;            // stop ignoring collisions if there are none

        collisions = 0;
    }
    else
        coll = 0;

    return coll;
}

//
// TestCollision(item)
//
bool SS_Collider::TestCollision(SS_Collider *other)
{
    if ( CanCollide(other) )
    {
        collisionUpdated = other->collisionUpdated = true;

        if (DistanceSquaredTo(other) > SS_COLLISION_BAND_SIZE*SS_COLLISION_BAND_SIZE)
            return false;
        else
            return _TestCollision(other);
    }
    else
        return false;
}


