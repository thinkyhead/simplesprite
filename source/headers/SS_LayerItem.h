/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_LayerItem.h
 *
 *  $Id: SS_LayerItem.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_LAYERITEM_H__
#define __SS_LAYERITEM_H__

#include <math.h>
#include <string.h>

#include "SS_Types.h"
#include "SS_RefCounter.h"
#include "SS_Messages.h"
#include "SS_Utilities.h"

// Sprite Proc Pointer
typedef void (*spriteProcPtr)(SS_LayerItem*);

enum itemType {
    SS_ITEM_PLAIN = 0,
    SS_ITEM_SPRITE,
    SS_ITEM_GROUP,
    SS_ITEM_GADGET,
    SS_ITEM_PROC,
    SS_ITEM_STRING
};

// Helpful Typedefs
class SS_LayerItem;
typedef TListNode<SS_LayerItem*>    SS_ItemNode;
typedef TIterator<SS_LayerItem*>    SS_ItemIterator;

typedef TArray<SS_ItemNode*>        SS_ItemNodeArray;

//--------------------------------------------------------------
// SS_LayerItem
//
// A sprite or spritegroup
//
#define SS_MISC_FIELDS  20

#pragma mark -
class SS_LayerItem : public SS_Broadcaster, public SS_Listener, public SS_RefCounter
{
    friend class SS_Sprite;
    friend class SS_ItemGroup;
    friend class SS_Layer;
    friend class SS_World;

    protected:
        SS_World                *world;                     // the world
        SS_Layer                *layer;                     // the layer
        SS_ItemGroup            *group;                     // the group

        SS_ItemNodeArray        nodeArray;                  // all nodes referencing this item

        // Flags
        Uint32                  flags;                      // the sprite's flags, as above
        bool                    removeFlag, hideFlag;       // flags to delete and hide

        // Lifespan
        Uint16                  lifespan;                   // number of cycles until death
//      float                   fuseAmount;                 // fuse could be expressed as a fraction of 1.0

        // Movement
        Uint32                  lastMoveTime;               // last time the moveProc was called
        spriteProcPtr           moveProc;                   // the sprite's move proc

        // Animation
        Uint32                  lastAnimTime;               // last time the animProc was called
        spriteProcPtr           animProc;                   // the sprite's animation proc
        Uint16                  frameCount;                 // total number of frames added
        Uint16                  currFrame;                  // index of frame to display
        Uint16                  animFirst;                  // first frame in auto-animation
        Uint16                  animLast;                   // last frame in auto-animation
        Sint16                  animIncrement;              // amount to add to currFrame
        Uint32                  animInterval;               // how often to call the animProc

        // Zoom compensation
        float                   oldW, oldH;                 // the world's view size at creation time

    public:
        Uint32                  moveInterval;               // how often to call the moveProc

        float                   xpos, ypos;                 // the sprite's position in the world or group
        float                   xvel, yvel;                 // the sprite's velocity
        float                   rotation;                   // the sprite's rotation
        Uint16                  rotindex;                   // the rotation as an index
        float                   spin;                       // the sprite's spin
        float                   xscale, yscale;             // the sprite's scaling factors
        float                   xhandle, yhandle;           // the sprite's handle offset
        float                   width, height;              // estimated size
        SScolorb                tint;                       // a color to tint the sprite

        // TODO: Kinda presumptuous - eventually migrate these to a game class
        float                   radarSize;                  // size of the blip - default 2
        SScolorb                radarColor;                 // yep, built-in radar support

        float                   misc[SS_MISC_FIELDS];       // whatever
        bool                    yesno[SS_MISC_FIELDS];      // whatever

        char                    *name;

    public:
                                SS_LayerItem();
                                SS_LayerItem(const SS_LayerItem &src) { Init(); *this = src; }
        virtual                 ~SS_LayerItem();

        virtual const SS_LayerItem&     operator=(const SS_LayerItem &src);
        virtual SS_LayerItem*           Clone() { return new SS_LayerItem(*this); }

        virtual itemType        Type() const { return SS_ITEM_PLAIN; }
        virtual void            Tokenize(SS_FlatFile &dataFile);

        inline void             Kill() { removeFlag = true; }
        virtual void            RemoveSelf();

        void                    AddPeer(SS_LayerItem *item) const;
        void                    PrependPeer(SS_LayerItem *item) const;

        // Accessors
        inline SS_World*        World() const           { return world; }
        inline SS_Layer*        Layer() const           { return layer; }
        inline SS_ItemGroup*    Group() const           { return group; }
        inline Uint32           Flags() const           { return flags; }
        inline Uint32           Flags(Uint32 m) const   { return flags & m; }
        virtual bool            IsVisible() const;
        inline Uint16           FrameCount() const      { return frameCount; }

        // Setters
        virtual void            SetWorld(SS_World *w);
        virtual void            SetLayer(SS_Layer *l);
        virtual void            SetGroup(SS_ItemGroup *g);
        inline void             SetName(char *n)        { if (name) delete[] name; name = newstring(n); }

        virtual inline void     SetFlags(Uint32 f)      { flags = f; }
        virtual inline void     EnableFlag(Uint32 f)    { flags |= f; }
        virtual inline void     DisableFlag(Uint32 f)   { flags &= ~f; }
        virtual inline void     SetHidden(bool h)       { hideFlag = h; }
        inline void             Hide()                  { SetHidden(true); }
        inline void             Show()                  { SetHidden(false); }

        inline void             SetTint(const SScolorb &t) { tint = t; }
        inline void             SetTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { tint.r = r; tint.g = g; tint.b = b;  tint.a = a; }
        inline void             SetTint(GLubyte r, GLubyte g, GLubyte b) { tint.r = r; tint.g = g; tint.b = b; }
        inline void             SetAlpha(GLubyte a) { tint.a = a; }
        virtual inline void     SetRadarColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { radarColor.r = r;  radarColor.g = g;  radarColor.b = b;  radarColor.a = a; }
        inline void             SetRadarColor(GLubyte r, GLubyte g, GLubyte b) { SetRadarColor(r, g, b, 0xFF); }
        inline void             SetRadarSize(float s) { radarSize = s; }

        virtual inline void     SetFrameIndex(Uint16 f) { currFrame = f % frameCount; }
        virtual inline void     SetAnimRange(Uint16 start, Uint16 end) { if (end < start) end = start; animFirst = start; animLast = end; }
        inline void             SetAnimInterval(Uint32 ms) { animInterval = 0; lastAnimTime = 0; animInterval = ms; }
        inline void             SetAnimIncrement(Sint16 inc) { animIncrement = inc; }
        void                    SetProcessInterval(Uint32 ms);
        void                    PrimeProcessor();
        void                    SetAnimateProc(spriteProcPtr proc);
        void                    SetMoveProc(spriteProcPtr proc);

        virtual void            PushAndPrepareMatrix();                 // Set the matrix for this container
        inline void             RestoreMatrix() { glPopMatrix(); }      // Restore the old matrix

        inline Uint16           BurnFuse() { if (lifespan && --lifespan <= 0) { Kill(); } return lifespan; }
        inline void             LightFuse(Uint16 fuse) { lifespan = fuse; }

        virtual inline void     SetHandle(float x, float y) { xhandle = x; yhandle = y; }
        virtual inline void     CenterHandle() { SetHandle(width / 2, height / 2); }

        virtual inline void     Move(float x, float y) { xpos = x; ypos = y; }
        void                    Move(float x, float y, float h, float v, float a);
        inline void             Offset(float h, float v) { Move(xpos + h, ypos + v); }
        virtual inline void     ApplyMotion() { Offset(xvel, yvel);  }
        virtual inline void     AutoMove() { ApplyMotion(); SetRotation(rotation + spin); }
        virtual void            Center();

        inline float            Heading() const { return DEG(HeadingRad()); }
        inline float            HeadingRad() const { float d = xvel, v = 0; if (d) { v = atan(yvel/d) + (M_PI / 2); if (d < 0) v -= M_PI; if (v < 0) v += 2 * M_PI; }; return v; }
        inline float            VelocitySquared() const { return xvel*xvel + yvel*yvel; }
        inline float            Velocity() const { return sqrt(VelocitySquared()); }
        virtual inline void     SetVelocity(float h, float v) { xvel = h; yvel = v; }
        inline void             LimitVelocity(float cap) { if (Velocity() > cap) SetAngularVelocity(Heading(), cap); }
        virtual void            SetAngularVelocity(float a, float vel);
        void                    HeadTowards(float ang, float rate=1.0f);
        inline void             HeadTowards(SS_LayerItem *item, float rate=1.0f) { HeadTowards(AngleTo(item), rate); }

        virtual inline void     SetSpin(float s) { spin = s; }

        virtual void            SetGlobalRotation(float deg);
        inline void             SetGlobalRotationRad(float rad) { SetGlobalRotation(DEG(rad)); }

        virtual inline void     SetRotation(float deg) { rotation = deg; rotindex = SS_ROTINDEX(deg); }
        inline void             SetRotationRad(float rad) { SetRotation(DEG(rad)); }

        virtual inline void     SetScale(float sx, float sy)    { xscale = sx; yscale = sy; }
        inline void             SetScale(float xy)              { SetScale(xy, xy); }
        inline void             SetWidth(float w)               { xscale = w / width; }
        inline void             SetHeight(float h)              { yscale = h / height; }

        virtual bool            IsOnScreen() const { printf("root isonscreen()\n"); return true; }

        double                  DistanceSquaredTo(SS_LayerItem * const other) const;
        double                  DistanceSquaredTo(float x2, float y2) const;

        inline double           DistanceTo(SS_LayerItem * const other) const { return sqrt(DistanceSquaredTo(other)); }
        inline double           DistanceTo(float x2, float y2) const { return sqrt(DistanceSquaredTo(x2, y2)); }

        virtual void            GlobalPosition(SS_Point * const point, bool scr=false) const;

        virtual void            GlobalVelocity(float *xv, float *yv) const;
        virtual float           GlobalVelocity() const { float xv, yv; GlobalVelocity(&xv, &yv); return sqrt(xv*xv+yv*yv); }

        virtual void            LocalPosition(SS_Point * const point) const;
        virtual float           LocalAngle(float globalAngle) const;
        inline void             LayerPosition(SS_Point * const point) const { GlobalPosition(point, true); }

        float                   AngleTo(float x, float y) const;
        float                   AngleTo(SS_LayerItem * const other) const;
        float                   AngleToPointer() const;

        virtual void            BounceOff(SS_LayerItem * const base);

        virtual void            _Process();
        virtual void            Process();
        void                    _Animate();
        virtual void            Animate();
        void                    Render() { Render(SS_WHITE_B); }
        virtual void            Render(const SScolorb &inTint);

        // Events and Commands
        virtual bool            HandleEvent(SDL_Event *e)       { return false; }
        virtual bool            HandleEvent(SS_Event *e)        { return false; }
        virtual void            SendCommand(long command);

        virtual bool            TestPointCollision(SS_LayerItem *other);
        virtual bool            TestPointCollision(SS_Point *globalPoint);
        virtual bool            TestPointCollision(float x, float y, bool isLocal=false);

        static void             defaultAnimProc(SS_LayerItem *item);

    private:
        void                Init();
};

#endif

