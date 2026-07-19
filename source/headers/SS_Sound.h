/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Sound.h
 *
 *  $Id: SS_Sound.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 *  SDL3 / SDL3_mixer (3.x) port: the old SDL2_mixer Mix_Chunk API was removed
 *  in SDL3_mixer, replaced by a MIX_Mixer / MIX_Audio / MIX_Track model.
 *  SS_Sound now wraps one MIX_Audio (decoded sample) plus one MIX_Track
 *  (a playback voice) created on demand. SS_Music reuses the exact same
 *  machinery with a dedicated track. See SS_Sound.cpp for the mapping.
 */

#ifndef __SS_SOUND_H__
#define __SS_SOUND_H__

#include <SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

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
        MIX_Audio   *soundChunk;        // decoded sample (was Mix_Chunk*)
        MIX_Track   *soundTrack;        // playback voice (created lazily)

    public:
                    SS_Sound(Uint32 min=0);
                    SS_Sound(const char *filename, Uint32 min=0);
                    ~SS_Sound();

        static void InitAudioMixer(Uint32 channels);

        static void Stop(int channel)                       { if (channel>=0) MixHalt(channel); }
        static void Pause(int channel)                      { if (channel>=0) MixPause(channel); }
        static void Resume(int channel)                     { if (channel>=0) MixResume(channel); }
        static void FadeOut(int channel, int ms)            { if (channel>=0) MixFadeOut(channel, ms); }
        static void SetVolume(int channel, int vol)         { if (channel>=0) MixVolume(channel, vol); }
        static void SetPanning(int channel, Uint8 pan)      { if (channel>=0) MixPan(channel, pan); }
        static bool IsChannelPlaying(int channel)           { return (channel>=0) ? MixPlaying(channel) : false; }

        inline void SetMinWait(Uint32 min)                  { minWait = min; }

        void        Load(const char *filename);

        int         Play(int times=0);
        int         PlayAtPosition(Sint16 angle, Uint8 distance, int loops=0);

        void        Dispose();


    private:
        void        Init();

        // Static helpers map old "channel number" calls onto the live track set.
        // In the new model a "channel" is just an index into the active tracks;
        // we keep the old integer-channel API so game code is untouched.
        static void MixHalt(int channel);
        static void MixPause(int channel);
        static void MixResume(int channel);
        static void MixFadeOut(int channel, int ms);
        static void MixVolume(int channel, int vol);
        static void MixPan(int channel, Uint8 pan);
        static bool MixPlaying(int channel);
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
        MIX_Audio   *music;             // decoded music data (was Mix_Music*)
        MIX_Track   *musicTrack;        // dedicated music playback voice

    public:
                    SS_Music();
                    SS_Music(const char *filename);
        virtual     ~SS_Music();

        void        Load(const char *filename);

        inline int  Play(int loops=-1)          { return MusicPlay(musicTrack, music, loops, 0); }
        inline int  FadeIn(int loops, int ms)   { return MusicPlay(musicTrack, music, loops, ms); }
        void        FadeOut(int ms);
        inline int  SetVolume(int vol)          { if (musicTrack) MIX_SetTrackGain(musicTrack, vol/128.0f); return vol; }

        void        Pause();
        void        Stop();

        void        Dispose()                   { if (music) { MIX_DestroyAudio(music); music=NULL; } if (musicTrack) { MIX_DestroyTrack(musicTrack); musicTrack=NULL; } }

        void        SetVolume(Uint32 vol);
        void        SetRate(Uint32 rate);
        void        SetFrequency(int freq);

    private:
        void        Init();
        static int  MusicPlay(MIX_Track *&track, MIX_Audio *audio, int loops, int fadeMs);
};

#endif
