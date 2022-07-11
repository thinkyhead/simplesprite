/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Layer.cpp
 *
 *  $Id: SS_Layer.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Layer.h"
#include "SS_LayerItem.h"

#include "SS_Types.h"
#include "SS_World.h"
#include "SS_Game.h"


//--------------------------------------------------------------
// SS_World ... Layer methods
//--------------------------------------------------------------

//
// NewLayer
// Create a layer, add it to the world, and return the layer pointer
//
SS_Layer* SS_World::NewLayer(Uint32 f)
{
    DEBUGF(1, "[%08X] SS_World::NewLayer(%04X)\n", this, f);

    SS_Layer *layer = new SS_Layer(this, f);
    return layer;
}


//
// AddLayer(layer)
// Add a layer to the world
//
void SS_World::AddLayer(SS_Layer *layer)
{
    DEBUGF(1, "[%08X] SS_World::AddLayer(%08X)\n", this, layer);

    layer->SetWorld(this);
    Append(layer);

    if (pointerSprite != NULL)
        LayerToFront(pointerSprite->Layer());
}


//
// LayerToFront(layer)
//
void SS_World::LayerToFront(SS_Layer *layer)
{
    Remove(layer);
    Append(layer);

    if (pointerSprite != NULL && layer != pointerSprite->Layer())
        LayerToFront(pointerSprite->Layer());
}


//
// LayerToBack(layer)
//
void SS_World::LayerToBack(SS_Layer *layer)
{
    Remove(layer);
    Prepend(layer);
}


#pragma mark -
//--------------------------------------------------------------
// SS_Layer
// A basic layer in the world
//--------------------------------------------------------------

SS_Layer::SS_Layer(Uint32 f)
{
    DEBUGF(1, "[%08X] SS_Layer(%04X) CONSTRUCTOR\n", this, f);

    Init(f);
}


SS_Layer::SS_Layer(SS_World *w, Uint32 f)
{
    DEBUGF(1, "[%08X] SS_Layer(%08X, %04X) CONSTRUCTOR\n", this, w, f);

    Init(f);
    w->AddLayer(this);
}


SS_Layer::~SS_Layer()
{
    DEBUGF(1, "[%08X] ~SS_Layer() DESTRUCTOR\n", this);

    RemoveSelf();
}


void SS_Layer::RemoveSelf()
{
    world->Remove(this);
}


//
// Init(flags)
//
void SS_Layer::Init(Uint32 f)
{
    DEBUGF(1, "[%08X] SS_Layer::Init(flags)\n", this);

    Clear();
    visibleList.Clear();

    flags       = f;
    enabled     = true;
    paused      = false;
    removeFlag  = false;

    world       = NULL;

    spatialScale = 1.0f;

    xoffset = 0.0f;
    yoffset = 0.0f;

    SetTint(0xFF, 0xFF, 0xFF, 0xFF);
}

//
// SetWorld
// Set the world for all sprites and groups in this group
//
void SS_Layer::SetWorld(SS_World *w)
{
    DEBUGF(1, "[%08X] SS_Layer::SetWorld(%08X)\n", this, w);

    world = w;

    SS_LayerItem    *item;
    SS_ItemIterator itr = GetIterator();
    while (item = itr.NextItem())
        item->SetWorld(w);
}

//
// AddItem(layeritem)
// Add a layeritem to the layer
//
void SS_Layer::AddItem(SS_LayerItem *item)
{
    DEBUGF(1, "[%08X] SS_Layer::AddItem(item)\n", this);

    item->SetLayer(this);
    item->SetHidden(false);

    item->Retain("Item in layer");
    if (item->RefCount() > 1)
        printf("Over 1\n");

    Append(item);
}

//
// PrependItem(layeritem)
// Add a layeritem to the front (first-drawn) layer position
//
void SS_Layer::PrependItem(SS_LayerItem *item)
{
    DEBUGF(1, "[%08X] SS_Layer::PrependItem(item)\n", this);

    item->SetLayer(this);
    item->SetHidden(false);

    item->Retain("Item in layer");
    if (item->RefCount() > 1)
        printf("Over 1\n");

    Prepend(item);
}

//
// Process
// Tell all the items in the layer to Process
//
void SS_Layer::Process()
{
    SS_LayerItem    *item;
    SS_ItemIterator itr = GetIterator();
    while (item = itr.NextItem())
    {
        if (item->removeFlag)
            item->RemoveSelf();
        else {
            item->_Process();
            item->_Animate();
        }
    }
}

//
// Animate
// Tell all the items in the layer to Animate
//
void SS_Layer::Animate()
{
    visibleList.Clear();

    SS_LayerItem    *item;
    SS_ItemIterator itr = GetIterator();
    while (item = itr.NextItem()) {
        item->_Animate();
        if (item->IsOnScreen())
            AddToVisible(item);
    }
}

//
// Render
// Tell all the items in the layer to Render
//
void SS_Layer::Render()
{
    PrepareMatrix();

    SS_LayerItem    *item;
    SS_ItemIterator itr = visibleList.GetIterator();
    while (item = itr.NextItem())
        item->Render(tint);
}

//
// RemoveItem
// Remove a single item from the layer
// EXPENSIVE! DO NOT USE!
//
void SS_Layer::RemoveItem(SS_LayerItem *item)
{
    DEBUGF(1, "[%08X] SS_Layer::RemoveSprite(%08X)\n", this, item);

    visibleList.Remove(item);
    SS_ItemList::RemoveItem(item);
}

//
// DisposeItem
//
//  Locate and dispose a single item in the layer.
//  You probably shouldn't use this method, but here it is.
//
//  SLOW - Uses a Linked list search to find the item
//
//  TODO: Save and use the item's node instead if removal of this kind is needed,
//  or Kill() the item and it will be removed during processing.
//
void SS_Layer::DisposeItem(SS_LayerItem *item)
{
    DEBUGF(1, "[%08X] SS_Layer::DisposeItem(%08X)\n", this, item);

    visibleList.Remove(item);
    delete item;
}

//
// PrepareMatrix
// Prepare the projection matrix, either zooming it or not
//
void SS_Layer::PrepareMatrix()
{
    float   x, y, w, h;

    if (flags & SS_NOSCROLL) {
        x = -xoffset;
        y = -yoffset;
    }
    else {
        x = world->left;
        y = world->top;
    }

    if (flags & SS_NOZOOM) {
        w = world->ViewWidth();
        h = world->ViewHeight();
    }
    else {
        w = world->ZoomWidth();
        h = world->ZoomHeight();
    }

    gl_bind_texture(0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(x, x + w, y + h, y, -1.0f, 1.0f);
}

