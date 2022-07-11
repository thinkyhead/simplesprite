/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_ItemGroup.cpp
 *
 *  $Id: SS_ItemGroup.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_ItemGroup.h"

#include "SS_Layer.h"
#include "SS_Sprite.h"
#include "SS_Utilities.h"


//--------------------------------------------------------------
// SS_ItemGroup
// A manipulable object consisting of one or more sub-sprites
//--------------------------------------------------------------

SS_ItemGroup::SS_ItemGroup()
{
    Init();
}

void SS_ItemGroup::Init()
{
    Clear();
}


//
// SetWorld
// Set the world for all sprites and groups in this group
//
void SS_ItemGroup::SetWorld(SS_World *w)
{
    DEBUGF(1, "[%08X] SS_ItemGroup::SetWorld(%08X)\n", this, w);

    SS_Collider::SetWorld(w);

    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        item->SetWorld(w);
}


//
// SetLayer
// Set the layer for all sprites and groups in this group
//
void SS_ItemGroup::SetLayer(SS_Layer *l)
{
    SS_Collider::SetLayer(l);

    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        item->SetLayer(l);
}


//
// SetHidden
// Hide every sprite (and group) in the group
//
void SS_ItemGroup::SetHidden(bool h)
{
    DEBUGF(1, "[%08X] SS_ItemGroup::SetHidden(%d)\n", this, h);

    hideFlag = h;

/*
    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        item->SetHidden(h);
*/
}


//
// AddItem
// Add an item (which may be a group) to the group
//
// By telling an item it belongs to a certain world
// it should update its collider situation with regard
// to that world and the world it might have previously
// occupied.
//
// When setting an item's collision bits it maintains
// its own membership in the collision list. This should
// be revised. The world should be asked to do the collision
// list maintenance, or the collision list itself should
// become an object.
//
// TODO: Account for colliders!
//
//  [Q] Can both SS_LayerItem* and SS_Collider* be employed?
//
void SS_ItemGroup::AddItem(SS_Collider *item)
{
    DEBUGF(1, "[%08X] SS_ItemGroup::AddItem(%08X)\n", this, item);

    item->SetGroup(this);

    if (item->width > width)
        width = item->width;

    if (item->height > height)
        height = item->height;

    item->Retain("Item In Group");
    Append(item);

    //
    // Next, if this group is in a layer already then
    // see if the item should be added to the collider list
    // and then add it.
    //
    // 8-17-05 : Disabled because SetGroup() automatically maintains colliders
    //
//  if (layer && (item->CollisionOut() || item->CollisionIn()) && !item->IsCollider())
//      item->AddToColliders();

    item->Show();
}

//
// operator=
//
const SS_ItemGroup& SS_ItemGroup::operator=(const SS_ItemGroup &src)
{
    if (&src != this)
    {
        SS_Collider::operator=(src);

        SS_Collider     *item;
        SS_ColliderIterator itr = src.GetIterator();
        while ((item = itr.NextItem()))
            AddItem(item->Clone());
    }

    return *this;
}

//
// Process
//
// Process the group and "_Process" the member items.
//
//  Member items may or may not receive Process() but they
//  will definitely AutoMove and have their collision nodes
//  updated.
//
//  UpdateNodePosition used to be overridden for this class
//  but since only sprites will actually have nodes and the
//  migration of nodes is now tied directly to _Process the
//  concept of group->UpdateNodePosition() no longer makes sense.
//
void SS_ItemGroup::Process()
{
    SS_Collider::Process();     // Calls the base class's proc pointer

    SS_Collider *item;
    SS_ColliderIterator itr = GetIterator();
    while (itr.IsValid())
    {
        item = itr.Item();

        if (item->removeFlag)
        {
            // Dispose the node and the sub-item.
            // (Also moves the iterator forward)
            // The collider node will be removed during destruction.
            Dispose(itr);
        }
        else {
            item->_Process();
            itr.Next();
        }
    }
}


//
// Animate
// Animate every sprite (and group) in the group
//
void SS_ItemGroup::Animate()
{
    SS_Collider::Animate();     // Calls the base class's proc pointer

    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        item->_Animate();
}


//
// AutoMove
// Auto-move every sprite (and group) in the group
//
void SS_ItemGroup::AutoMove()
{
    SS_Collider::AutoMove();

    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        item->AutoMove();
}


//
// PushAndPrepareMatrix
// Save the current model matrix and transform it into our local coordinate system
//
void SS_ItemGroup::PushAndPrepareMatrix()
{
    SS_Collider::PushAndPrepareMatrix();
    glTranslatef(-xhandle, -yhandle, 0.0f);
}

//
// Render
// Render every sprite (and group) in the group
//
void SS_ItemGroup::Render(const SScolorb &inTint)
{
    if (!removeFlag && IsVisible())
    {
        SScolorb    mulTint;
        MultiplyColorQuads(inTint, tint, mulTint);
        PushAndPrepareMatrix();

        SS_Collider     *item;
        SS_ColliderIterator itr = GetIterator();
        while ((item = itr.NextItem()))
            item->Render(mulTint);

        RestoreMatrix();
    }
}


//
// IsOnScreen
// Determine if any sub-sprite is on-screen
//
bool SS_ItemGroup::IsOnScreen()
{
    DEBUGF(1, "SS_ItemGroup::IsOnScreen()\n");

    if (!removeFlag && IsVisible())
    {
        SS_Collider     *item;
        SS_ColliderIterator itr = GetIterator();
        while ((item = itr.NextItem()))
        {
            if (item->IsOnScreen())
                return true;
        }
    }

    return false;
}


#pragma mark -
//
// EnableCollisions(out, in)
// Set which collisions to accept and generate
//
void SS_ItemGroup::EnableCollisions(Uint32 out, Uint32 in)
{
    DEBUGF(1, "[%08X] SS_ItemGroup::EnableCollisions(%04X, %04X)\n", this, out, in);

    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        item->EnableCollisions(out, in);
}

void SS_ItemGroup::SetCollisionOrigin(Uint16 n)
{
    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        item->SetCollisionOrigin(n);
}

/*
//
// UpdateNodePosition
// Move colliders around for the whole group
//
void SS_ItemGroup::UpdateNodePosition()
{
    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        item->UpdateNodePosition();
}
*/

//
// GetNewCollisions()
//
Uint32 SS_ItemGroup::GetNewCollisions()
{
    DEBUGF(1, "[%08X] SS_ItemGroup::GetNewCollisions()\n", this);

    Uint32  coll = 0;

    SS_Collider     *item;
    SS_ColliderIterator itr = GetIterator();
    while ((item = itr.NextItem()))
        coll |= item->GetNewCollisions();

    if (coll)
    {
        coll &= ~collisionIgnore;
        collisionIgnore |= coll;
    }
    else
        collisionIgnore = 0;

    collisions = 0;
    return coll;
}

