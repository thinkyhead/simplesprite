/*
 *  SimpleSpriteApp "Hello SS_World" demo code
 *
 *  My_Game.cpp
 *
 *  $Id: My_Game.cpp,v 1.1 2007/03/02 08:09:25 slurslee Exp $
 *
 */

#include "My_Game.h"

//
// SampleGUI
//
class SampleGUI : public SS_GUI
{
	public:
		SampleGUI() : SS_GUI(new SS_SFont("font/verdana11.png", 3))
		{
		}
};

//
// SampleWorld Constructor
//
class SampleWorld : public SS_World
{
	public:

		SampleWorld()
		{
			SS_Folder::cdDataFolder();

			// +++ Make a Text Layer
			SS_TextLayer	*layer = NewTextLayer("font/verdana11.png", 3);
			SS_String		*helloString = layer->Print("Hello World", SS_VIDEO_W / 2, SS_VIDEO_H / 2);
			helloString->SetAlignment(SA_CENTER);
			helloString->SetFlags(SS_AUTOMOVE);
			helloString->SetMoveProc(StringMoveProc);
			helloString->SetProcessInterval(1000/10);
			helloString->SetAngularVelocity(RANDFLOAT(0, 360), RANDFLOAT(1, 10));
		}

		//
		// HandleEvent
		//
		void HandleEvent(SDL_Event *event)
		{
			switch (event->type)
			{
				case SDL_KEYDOWN:

					switch ((int)event->key.keysym.sym)
					{
						case SDLK_ESCAPE:
							Quit();
							break;

						default:
							break;
					}
					break;
			}
		}

		static void StringMoveProc(SS_LayerItem *item)
		{
			if (RANDINT(0, 70) == 0)
				item->SetAngularVelocity(RANDFLOAT(0, 360), RANDFLOAT(1, 10));

			if ( (item->xpos < 0 && item->xvel < 0) || (item->xpos > SS_VIDEO_W && item->xvel > 0) )
				item->xvel *= -1;

			if ( (item->ypos < 0 && item->yvel < 0) || (item->ypos > SS_VIDEO_H && item->yvel > 0) )
				item->yvel *= -1;
		}
};


#pragma mark -
//--------------------------------------------------------------
// My_Game
//--------------------------------------------------------------

My_Game::My_Game()
{
//	smallFont = new SS_SFont("", 4);
}

//
// Run
//
void My_Game::Run()
{
	Uint32	fps;

	//
	// Instantiate our sample SS_World subclass
	//
	SampleWorld	*sampleWorld = new SampleWorld();
	fps = SetWorld(sampleWorld)->Run(this);
	delete sampleWorld;

	SDL_ShowCursor(true);

	fprintf(stderr, "\nFrame Rate: %d FPS\n", fps);
}

