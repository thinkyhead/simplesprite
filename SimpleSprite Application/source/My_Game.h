/*
 *  SimpleSpriteApp "Hello SS_World" demo code
 *
 *  My_Game.h
 *
 *  $Id: My_Game.h,v 1.1 2007/03/02 08:09:25 slurslee Exp $
 *
 */

#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include <SimpleSprite/SimpleSprite.h>

//
// My_Game Class
//
class My_Game : public SS_Game
{	
	public:
						My_Game();
						~My_Game() {}
	
		void			Run();
};

#endif

