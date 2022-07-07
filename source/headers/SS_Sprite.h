/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Sprite.h
 *
 *  $Id: SS_Sprite.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_SPRITE_H__
#define __SS_SPRITE_H__

#include "SS_Frame.h"
#include "SS_LayerItem.h"
#include "SS_Layer.h"

#pragma mark -
//--------------------------------------------------------------
// SS_Sprite
//
// A single graphical object containing one or more frames
//
// The old method to generate running sprites was to create a
// "master sprite" then use sprite->Clone() to make lots of copies of it
// However it's better C++ to make a copy constructor and override operator=.
// The copy constructor simply copies a source object's fields to a new object.
// Operator = does the same thing.
//
// Clone() is not entirely deprecated. Consider the case of an instance being
// represented in the data type of a base class. Using a copy-constructor will
// only get a copy of the base class. To get the whole object, Clone will do
// the right thing.
//
// TODO:Currently the SS_Sprite is the lowest form of life able to collide with
// other things. This is for reciprocal value. Testing is always the same
// between two sprites. There will be instances in some games where it will
// be desirable for vector objects to collide with sprites, or for particles
// to collide with things. Who knows? But I will need to design a mechanism
// that all colliding objects can share. Much thought needed, ugh.
//
//	TODO: Basic collision testing via overlap and radius can be handled by the
//	collision manager. Radius is the simplest thing to test. Compare the
//	hypotenuse squared (skip the sqrt step) to a given radius squared (r*r).
//
//		CollisionManager::PointInItemRegion(SDL_Point &pt, Collider *collItem)
//		CollisionManager::PointInItemGroup(SDL_Point &pt, SS_ItemGroup *group)
//		CollisionManager::PointInSprite(SDL_Point &pt, SS_Sprite *sprite)
//
#define SS_FRAME_BLOCK	5
class SS_Sprite : public SS_Collider
{
    protected:
		Uint16				frameBlocks;			// size of the allocated frame array
		SS_Frame			**frameArray;			// pointer to an array of frame pointers

    public:
							SS_Sprite();
							SS_Sprite(const SS_Sprite &src) { Init(); *this = src; }
							SS_Sprite(char *spriteFile);
		virtual				~SS_Sprite();

		virtual const SS_Sprite&	operator=(const SS_Sprite &src);
		virtual SS_Sprite*			Clone() { return new SS_Sprite(*this); }

		virtual itemType	Type() { return SS_ITEM_SPRITE; }

		void				Export(char *fileName);

		void				AddFrame(SS_Frame *frame);
		inline void			AddFrame(char *frameFile) { AddFrame(new SS_Frame(frameFile)); }
		inline void			AddFrame(char *frameFile, frameFlags f) { AddFrame(new SS_Frame(frameFile, f)); }
		inline SS_Frame*	Frame(Uint16 fr) { return frameArray[fr]; }

		virtual void		Render(const SScolorb &inTint);

		void				ReleaseFrames();

		// Setters
		void				SetWorld(SS_World *w);
		void				SetHandle(Uint16 frame, float x, float y);
		void				SetHandle(float x, float y);
		void				CenterHandle(Uint16 frame);
		void				CenterHandle();
		void				SetFrameIndex(Uint16 f);
		void				SetAnimRange(Uint16 start, Uint16 end);

		// Diagnostics
		virtual bool		IsOnScreen();

		// Collisions
		bool				TestPointCollision(float x, float y, bool isLocal=false);
		bool				_TestCollision(SS_Collider *other);


	private:
		void				Init();
};


#endif

