/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Layer.h
 *
 *  $Id: SS_Layer.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_LAYER_H__
#define __SS_LAYER_H__

#include <math.h>

#include <SDL.h>
#include "SS_Types.h"
#include "SS_Templates.h"
#include "SS_Messages.h"
#include "SS_LayerItem.h"
#include "SS_World.h"

enum layerType {
    SS_LAYER_PLAIN,
    SS_LAYER_SPRITE,
    SS_LAYER_TEXT,
    SS_LAYER_TILE,
    SS_LAYER_GUI
};

//--------------------------------------------------------------
// SS_Layer
// A basic layer
//
class SS_Layer : public SS_Broadcaster, public SS_Listener, public SS_ItemList
{
    protected:
        SS_World                *world;                         // the world this layer is in
        SScolorb                tint;

    public:
        SS_ItemList             visibleList;                    // everything that ought to be rendered

        Uint32                  flags;                          // every layer has flags
        bool                    enabled;                        // layer on/off
        bool                    paused;                         // layer paused temporarily?
        bool                    removeFlag;                     // layer scheduled for deletion?

        float                   spatialScale;                   // scale relative to the world
        float                   xoffset, yoffset;               // The offset of the layer, woohoo!

    public:
                                SS_Layer(Uint32 f=SS_NONE);
                                SS_Layer(SS_World *w, Uint32 f=SS_NONE);
        virtual                 ~SS_Layer();

        virtual inline layerType   Type() const         { return SS_LAYER_PLAIN; }

        // Accessors
        inline SS_World*        World() const           { return world; }
        inline Uint32           Flags() const           { return flags; }
        inline Uint32           Flags(Uint32 m) const   { return flags & m; }
        inline bool             IsEnabled() const       { return enabled; }
        inline SS_LayerItem*    MousePointer() const    { return world ? world->MousePointer() : NULL; }
        inline GLubyte          Alpha() const           { return tint.a; }
        inline SScolorb         Tint() const            { return tint; }

        // Setters
        virtual void            SetWorld(SS_World *w);
        virtual inline void     SetFlags(Uint32 f)          { flags = f; }
        inline void             SetSpatialScale(float s)    { spatialScale = s; }
        virtual inline void     SetTint(SScolorb &inTint)   { tint = inTint; }
        virtual inline void     SetTint(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { tint.r = r;  tint.g = g;  tint.b = b;  tint.a = a; }
        virtual inline void     SetTint(GLubyte r, GLubyte g, GLubyte b) { tint.r = r;  tint.g = g;  tint.b = b; }
        virtual inline void     SetAlpha(GLubyte a)         { tint.a = a; }
        virtual inline void     SetEnabled(bool e)          { enabled = e; }
        virtual inline void     SetPaused(bool p)           { paused = p; }

        inline void             Enable()                { SetEnabled(true); }
        inline void             Disable()               { SetEnabled(false); }
        inline void             Pause()                 { SetPaused(true); }
        inline void             Resume()                { SetPaused(false); }
        inline void             Kill()                  { removeFlag = true; }

        virtual inline void     SetOffset(float h, float v) { xoffset = h; yoffset = v; }

        void                    AddItem(SS_LayerItem *item);
        void                    PrependItem(SS_LayerItem *item);

        inline void             AddToVisible(SS_LayerItem *item) { visibleList.Append(item); }

        virtual void            RemoveItem(SS_LayerItem *item);
        void                    DisposeItem(SS_LayerItem *item);

        virtual void            Process();
        virtual void            Animate();
        virtual void            Render();

        virtual bool            HandleEvent(SDL_Event *e)       { return false; }
        virtual bool            HandleEvent(SS_Event *e)        { return false; }
        virtual void            HandleCommand(long command)     {}

        void                    PrepareMatrix();            // set the openGL context for this layer
        inline void             RemoveSelf();

    private:
        void                    Init(Uint32 f=SS_NONE);
};

#endif

