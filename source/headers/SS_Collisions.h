/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Collisions.h
 *
 *  $Id: SS_Collisions.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_COLLISIONS_H__
#define __SS_COLLISIONS_H__

#include "SS_LayerItem.h"

#define SS_COLLISION_TYPES 64

#pragma mark -
class SS_Collider;
typedef TObjectList<SS_Collider*>   SS_ColliderObjectList;
typedef TLinkedList<SS_Collider*>   SS_ColliderList;
typedef TListNode<SS_Collider*>     SS_ColliderNode;
typedef TIterator<SS_Collider*>     SS_ColliderIterator;


#pragma mark -
class SS_Collider : public SS_LayerItem
{
    friend class SS_CollisionManager;

    protected:
        SS_ColliderNode         *collNode;                  // node in the collision list

        // Collisions
        bool                    isCollider;                 // has been added to the collider list
        Uint32                  collisions;                 // collision state
        Uint32                  collisionOut;               // collisions this generates
        Uint32                  collisionIn;                // collisions this accepts
        bool                    collisionUpdated;           // flag that collisions have been updated
        Uint32                  collisionIgnore;            // collisions to ignore
        Uint16                  collisionSource;            // an id number not to collide with

    public:
        int                     listNumber;                 // which collision list am i in?

    public:
                                SS_Collider() { Init(); }
                                SS_Collider(const SS_Collider &src) { Init(); *this = src; }

        void                    SetWorld(SS_World *w);

        virtual const SS_Collider&      operator=(const SS_Collider &src);
        virtual SS_Collider*            Clone()                 { return new SS_Collider(*this); }

        virtual void            RemoveSelf();

        Uint16                  GetSpatialIndex();
        inline SS_ColliderNode* ColliderNode()                  { return collNode; }
        SS_ColliderList*        ColliderList();
        inline bool             IsCollider()                    { return isCollider; }
        inline Uint32           Collisions()                    { return collisions; }
        inline Uint32           Collisions(Uint32 m)            { return (collisions & m); }
        inline Uint32           CollisionOut()                  { return collisionOut; }
        inline Uint32           CollisionIn()                   { return collisionIn; }
        inline void             ResetCollisions()               { collisions = 0; }
        inline void             CollisionAdd(Uint32 m)          { collisions |= m; }
        inline void             CollisionAdd(SS_Collider *s)    { CollisionAdd(s->CollisionOut()); }
        virtual void            CollideWith(SS_Collider *s);
        inline bool             CanCollide(SS_Collider *s) {
            return (    IsVisible() && s->IsVisible()
                    &&  ((collisionOut & s->collisionIn) != 0)
                    &&  ((collisionIn & s->collisionOut) != 0)
                    &&  (collisionSource == 0 || s->collisionSource == 0 || collisionSource != s->collisionSource)
                    );
                }
        inline void             SetCollisionOrigin(Uint16 n)    { collisionSource = n; }

        virtual void            _Process();

        void                    EnableCollisions(Uint32 out, Uint32 in);
        void                    AddToColliders();
        void                    RemoveFromColliders();
        virtual void            UpdateNodePosition();
        void                    UpdateCollisions();
        Uint32                  GetNewCollisions();
        bool                    TestCollision(SS_Collider *other);
        virtual bool            _TestCollision(SS_Collider *other) { return false; }

//      virtual bool            TestPointCollision(float x, float y, bool isLocal=false);

    private:
        void                    Init();
};


#pragma mark -
class SS_CollisionManager
{
    friend class SS_Collider;

    private:
        SS_ColliderList         colliderList[SS_COLLISION_LISTS];
//      SS_ColliderList         colliderList[SS_COLLISION_TYPES][SS_COLLISION_LISTS];

    public:
        SS_CollisionManager();
        ~SS_CollisionManager()  {}

        SS_ColliderNode*    AddToColliders(SS_Collider *item);
        void                RemoveFromColliders(SS_Collider *item) { item->RemoveFromColliders(); }
        void                RunCollisionTest();
        SS_ColliderList*    CollidersAtPoint(float x, float y);
        SS_Collider*        FirstColliderAt(float x, float y);
        SS_Collider*        FirstColliderOnLine(float x1, float y1, float x2, float y2);
        void                DrawCollisionGraph();

    private:
        void                Init();
};

#endif

