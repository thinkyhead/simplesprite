/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Particles.h
 *
 *  $Id: SS_Particles.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_PARTICLES_H__
#define __SS_PARTICLES_H__

#include "SS_LayerItem.h"

//--------------------------------------------------------------
// SS_Emitter
// A particle-generator
//--------------------------------------------------------------

enum {
	SS_EMIT_SQUARE	= 0x00000001,
	SS_EMIT_RING	= 0x00000002,
	SS_EMIT_SPRITE	= 0x00000004
};

class SS_Emitter : public SS_LayerItem
{
	public:
		Uint32			emitterFlags;		// square / circle / outline / etc.
		Uint32			interval;
		Uint16			emitMinCount;		// minimum
		Uint16			emitMaxCount;
		float			innerWidth;			// emitter range info
		float			innerHeight;
		float			innerRadius;
		float			outerRadius;
		float			chaosFactor;		// randomness
		float			arcSize;			// directional arc for spraying
		float			minVelocity;		// initial speed variance
		float			maxVelocity;
		SS_LayerItem	*emitItem;			// a sprite to clone

				SS_Emitter();
		virtual	~SS_Emitter();

		void	DoEmissions();
		virtual void	Emit(float h, float v, float a, float d, float vx, float vy) = 0;

	private:
		void	Init();
};

//--------------------------------------------------------------
// SS_Sprayer
// Emit particles from a single point
//--------------------------------------------------------------

class SS_Sprayer : public SS_Emitter
{
};

class SS_Dropper : public SS_Emitter
{
/*
	drop particles near a central point
*/
};

class SS_Polygon : public SS_LayerItem
{
/*
	PrependPoint(x, y)
	AppendPoint(x, y)
	DoublePoints()
*/
};

class SS_Streamer : public SS_LayerItem
{
/*
	SetMaxLength(len)
	PrependStrip(x1, y1, x2, y2)
	AppendStrip(x1, y1, x2, y2)
	RemoveFirstStrip()
	RemoveLastStrip()
*/
};

class SS_Fantail : public SS_LayerItem
{
/*
	SetMaxLength(len)
	PrependPoint(x, y)
	AppendPoint(x, y)
	RemoveFirstPoint()
	RemoveLastPoint()
*/
};

class SS_TriPoly : public SS_LayerItem
{
/*
	SS_Tripoly(cx, cy)
	SetMaxLength(len)
	PrependPoint(x, y)
	AppendPoint(x, y)
	RemoveFirstPoint()
	RemoveLastPoint()
	DoublePoints()
*/
};

#endif