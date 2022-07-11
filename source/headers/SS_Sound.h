/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Sound.h
 *
 *  $Id: SS_Sound.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_SOUND_H__
#define __SS_SOUND_H__

#include <SDL.h>
#include <SDL_mixer.h>

#include "SS_RefCounter.h"


//--------------------------------------------------------------
// SS_Sound
// A sound that can be played
//--------------------------------------------------------------

enum {
    SND_REPEAT  = 1,
    SND_WAV     = 2
};

class SS_Sound : public SS_RefCounter
{
    protected:
        Uint32      flags;
        Uint32      minWait;            // minimum time before playing again
        Uint32      lastPlay;           // last time it was played
        Mix_Chunk   *soundChunk;

    public:
                    SS_Sound(Uint32 min=0);
                    SS_Sound(char *filename, Uint32 min=0);
                    ~SS_Sound();

        static void InitAudioMixer(Uint32 channels);

        static void Stop(int channel)                       { Mix_HaltChannel(channel); }
        static void Pause(int channel)                      { Mix_Pause(channel); }
        static void Resume(int channel)                     { Mix_Resume(channel); }
        static void FadeOut(int channel, int ms)            { (void)Mix_FadeOutChannel(channel, ms); }
        static void SetVolume(int channel, int vol)         { (void)Mix_Volume(channel, vol); }
        static void SetPanning(int channel, Uint8 pan)      { (void)Mix_SetPanning(channel, pan, 255 - pan); }
        static bool IsChannelPlaying(int channel)           { return Mix_Playing(channel); }

        inline void SetMinWait(Uint32 min)                  { minWait = min; }

        void        Load(char *filename);

        int         Play(int times=0);
        int         PlayAtPosition(Sint16 angle, Uint8 distance, int loops=0);

        void        Dispose();


    private:
        void        Init();
};

#pragma mark -

//--------------------------------------------------------------
// SS_Music
// A sound intended for use as music
//--------------------------------------------------------------

class SS_Music
{
    protected:
        Uint32      flags;
        Mix_Music   *music;

    public:
                    SS_Music();
                    SS_Music(char *filename);
        virtual     ~SS_Music();

        void        Load(char *filename);

        inline int  Play(int loops=-1)          { return Mix_PlayMusic(music, loops); }
        inline int  FadeIn(int loops, int ms)   { return Mix_FadeInMusic(music, loops, ms); }
        inline int  FadeOut(int ms)             { return Mix_FadeOutMusic(ms); }
        inline int  SetVolume(int vol)          { return Mix_VolumeMusic(vol); }

        void        Pause();
        void        Stop();

        void        Dispose()                   { if (music) Mix_FreeMusic(music); }

        void        SetVolume(Uint32 vol);
        void        SetRate(Uint32 rate);
        void        SetFrequency(int freq);

    private:
        void        Init();
};

#endif
