/*
 *  SimpleSprite "Hello SS_World" program
 *
 *  main.cpp
 *
 */

#include "My_Game.h"

My_Game *theGame = NULL;

int main(int argc, char *argv[])
{
	try
	{
		theGame = new My_Game();
		theGame->Run();
    }
    catch (const char* err)
	{
		fprintf(stderr, err, SDL_GetError());
	}

	if (theGame)
		delete theGame;

	fprintf (stderr, "\nExiting Cleanly...\n");

	return(0);
}

