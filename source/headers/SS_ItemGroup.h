/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_ItemGroup.h
 *
 *  $Id: SS_ItemGroup.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_ITEMGROUP_H__
#define __SS_ITEMGROUP_H__

#include "SS_Collisions.h"

#pragma mark -
//--------------------------------------------------------------
// SS_ItemGroup
// A manipulable object consisting of one or more sub-sprites
//
class SS_ItemGroup : public SS_Collider, public SS_ColliderObjectList
{
    public:
                SS_ItemGroup();
                SS_ItemGroup(const SS_ItemGroup &src) { *this = src; }
        virtual ~SS_ItemGroup() {}

        virtual const SS_ItemGroup&     operator=(const SS_ItemGroup &src);
        virtual SS_ItemGroup*           Clone() { return new SS_ItemGroup(*this); }

        virtual inline itemType Type() { return SS_ITEM_GROUP; }

        void            SetWorld(SS_World *w);      // Set the world for all contained items
        void            SetLayer(SS_Layer *l);      // Set the layer and world for all contained items
        void            SetHidden(bool h);          // Set hidden on all contained Sprites

        void            AddItem(SS_Collider *item);     // Add a LayerItem to the Group
//      inline void     Append(SS_LayerItem *item) { AddItem(item); }

        virtual bool    IsOnScreen();

        void            Process();
        void            Animate();
        void            AutoMove();
        void            PushAndPrepareMatrix();
        void            Render(const SScolorb &inTint);

        void            EnableCollisions(Uint32 out, Uint32 in);
        void            UpdateCollisions();
//      virtual void    UpdateNodePosition();
        void            SetCollisionOrigin(Uint16 n);
        Uint32          GetNewCollisions();

    private:
        void            Init();
};


#endif

