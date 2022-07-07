/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_ItemList.h
 *
 *  $Id: SS_ItemList.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_ITEMLIST_H__
#define __SS_ITEMLIST_H__

#include "SS_Templates.h"
#include "SS_LayerItem.h"

class SS_ItemList : public TObjectList<SS_LayerItem*>
{
    public:
        virtual         ~SS_ItemList() { ReleaseAll(); }
        virtual void    RemoveItem(SS_LayerItem *item);
        void            ReleaseAll();
};

#endif
