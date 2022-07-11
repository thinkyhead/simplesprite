/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_ItemList.cpp
 *
 *  $Id: SS_ItemList.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Types.h"
#include "SS_ItemList.h"
#include "SS_LayerItem.h"

void SS_ItemList::RemoveItem(SS_LayerItem *item)
{
    Remove(item);
    item->Release();
}

void SS_ItemList::ReleaseAll()
{
    SS_ItemIterator itr = GetIterator();
    SS_LayerItem    *item;
    while ((item = itr.NextItem()))
        item->Release();

    Clear();
}

