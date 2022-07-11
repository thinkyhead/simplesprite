/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Sound.cpp
 *
 *  $Id: SS_Sound.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Sound.h"

#include "SS_Files.h"
#include "SS_Game.h"


//--------------------------------------------------------------
// SS_Sound
//--------------------------------------------------------------

SS_Sound::SS_Sound(Uint32 min)
{
    DEBUGF(1, "[%08X] SS_Sound::SS_Sound()\n", this);

    Init();
    minWait = min;
}

SS_Sound::SS_Sound(char *filename, Uint32 min)
{
    DEBUGF(1, "[%08X] SS_Sound::SS_Sound(%s)\n", this, filename);

    Init();
    Load(filename);
    minWait = min;
}

SS_Sound::~SS_Sound()
{
    DEBUGF(1, "[%08X] SS_Sound::~SS_Sound()\n", this);

    Dispose();
}


//
// Init
//
void SS_Sound::Init()
{
    DEBUGF(1, "[%08X] SS_Sound::Init()\n", this);

    flags = 0;
    minWait = 0;
    lastPlay = 0;
}


//
// Load(filename)
//
void SS_Sound::Load(char *filename)
{
    DEBUGF(1, "[%08X] SS_Sound::Load(%s)\n", this, filename);

    char *full = SS_Folder::FullPath(filename);

    if ( ! (soundChunk = Mix_LoadWAV(full)) ) {
        printf("%s\n", SDL_GetError());
        throw ("Can't load sound file!");
    }

    flags |= SND_WAV;
}


//
// Play
//
int SS_Sound::Play(int loops)
{
    int chan = -1;
    Uint32  tick = SDL_GetTicks();

    if (tick > (lastPlay + minWait)) {
        chan = Mix_PlayChannel(-1, soundChunk, loops);
        lastPlay = tick;
    }

    return chan;
}


//
// PlayAtPosition
//
int SS_Sound::PlayAtPosition(Sint16 angle, Uint8 distance, int loops)
{
    int realChannel = Play(loops);

    if (realChannel >= 0)
        Mix_SetPosition(realChannel, angle, distance);

    return realChannel;
}


//
// Dispose
//
void SS_Sound::Dispose()
{
    if (soundChunk)
        Mix_FreeChunk(soundChunk);
}

#pragma mark Utility Functions
//
// InitAudioMixer
//
void SS_Sound::InitAudioMixer(Uint32 channels)
{
    static bool mixerInit = false;

    if (!mixerInit)
    {
        if ( Mix_OpenAudio(22050, AUDIO_S16MSB, 2, 512) )
            printf("Couldn't open audio: %s\n", SDL_GetError());

        (void)Mix_AllocateChannels(channels);
        mixerInit = true;
    }
}

#pragma mark -
//--------------------------------------------------------------
// SS_Music
//--------------------------------------------------------------

SS_Music::SS_Music()
{
    DEBUGF(1, "[%08X] SS_Music::SS_Music()\n", this);

    Init();
}

SS_Music::SS_Music(char *filename)
{
    DEBUGF(1, "[%08X] SS_Music::SS_Music(%s)\n", this, filename);

    Init();
    Load(filename);
}

SS_Music::~SS_Music()
{
    DEBUGF(1, "[%08X] SS_Music::~SS_Music()\n", this);

    Dispose();
}


//
// Init
//
void SS_Music::Init()
{
    DEBUGF(1, "[%08X] SS_Music::Init()\n", this);
}


//
// Load(filename)
//
void SS_Music::Load(char *filename)
{
    DEBUGF(1, "[%08X] SS_Music::Load(%s)\n", this, filename);

    char        *full = SS_Folder::FullPath(filename);

    if ( ! (music = Mix_LoadMUS(full)) ) {
        printf("%s\n", SDL_GetError());
        throw ("Can't load music file!");
    }
}


