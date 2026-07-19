/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Sound.cpp
 *
 *  $Id: SS_Sound.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 *  SDL3 / SDL3_mixer (3.x) port.
 *
 *  The old SDL2_mixer "Mix_Chunk" API (Mix_LoadWAV / Mix_PlayChannel /
 *  Mix_HaltChannel / Mix_SetPanning / ...) was REMOVED in SDL3_mixer 3.x.
 *  It was replaced by a mixer/track model:
 *
 *      MIX_Mixer  - one global mixing engine (created from an SDL_AudioSpec)
 *      MIX_Audio  - one decoded sample (the old "Mix_Chunk"), reusable
 *      MIX_Track  - one playback voice (the old "channel"); assign an
 *                   MIX_Audio to it and MIX_PlayTrack() to sound it.
 *
 *  So SS_Sound now holds an MIX_Audio* (the sample) plus an MIX_Track*
 *  (its voice, created lazily on first Play). The old integer "channel"
 *  API is preserved for game code by keeping a global registry that maps
 *  channel numbers to live MIX_Track pointers.
 */

#include "SS_Sound.h"

#include "SS_Files.h"
#include "SS_Game.h"

#include <vector>
#include <algorithm>


//--------------------------------------------------------------
// Module state
//--------------------------------------------------------------

static MIX_Mixer   *g_mixer   = NULL;     // the one global mixer
static SDL_AudioSpec g_spec;              // cached mixer format (for ms<->frames)
static std::vector<MIX_Track*> g_channels; // channel index -> live track


// Helper: convert milliseconds to mixer frames (for fades / stop fades).
static Sint64 ms_to_frames(Uint32 ms)
{
    if (g_spec.freq == 0) return (Sint64)ms;   // fallback if spec unknown
    return (Sint64)((double)ms * g_spec.freq / 1000.0);
}

// Helper: build an SDL_PropertiesID for MIX_PlayTrack with loop + fade-in.
// FUTURE: pass MIX_PROP_PLAY_START_FRAME / _MAX_FRAME / _APPEND_SILENCE etc.
static SDL_PropertiesID make_play_props(int loops, int fadeMs)
{
    SDL_PropertiesID props = SDL_CreateProperties();
    if (props) {
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, (Sint64)loops);
        if (fadeMs > 0)
            SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, (Sint64)fadeMs);
    }
    return props;
}


//--------------------------------------------------------------
// SS_Sound
//--------------------------------------------------------------

SS_Sound::SS_Sound(Uint32 min)
{
    DEBUGF(1, "[%p] SS_Sound::SS_Sound()\n", this);

    Init();
    minWait = min;
}

SS_Sound::SS_Sound(char *filename, Uint32 min)
{
    DEBUGF(1, "[%p] SS_Sound::SS_Sound(%s)\n", this, filename);

    Init();
    Load(filename);
    minWait = min;
}

SS_Sound::~SS_Sound()
{
    DEBUGF(1, "[%p] SS_Sound::~SS_Sound()\n", this);

    Dispose();
}


//
// Init
//
void SS_Sound::Init()
{
    DEBUGF(1, "[%p] SS_Sound::Init()\n", this);

    flags = 0;
    minWait = 0;
    lastPlay = 0;
    soundChunk = NULL;
    soundTrack  = NULL;
}


//
// Load(filename)
//
void SS_Sound::Load(char *filename)
{
    DEBUGF(1, "[%p] SS_Sound::Load(%s)\n", this, filename);

    char *full = SS_Folder::FullPath(filename);

    // MIX_LoadAudio replaces Mix_LoadWAV. predecode=true so playback is
    // cheap (the sample is fully decoded up front, just like Mix_Chunk).
    if ( ! (soundChunk = MIX_LoadAudio(g_mixer, full, true)) ) {
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
    Uint32  tick = (Uint32)SDL_GetTicks();

    if (tick > (lastPlay + minWait)) {
        // Lazily create this sound's playback voice.
        if (!soundTrack) {
            soundTrack = MIX_CreateTrack(g_mixer);
            if (soundTrack)
                MIX_SetTrackAudio(soundTrack, soundChunk);
        }
        if (soundTrack) {
            MIX_SetTrackLoops(soundTrack, loops);   // -1 = forever (old behavior)
            SDL_PropertiesID props = make_play_props(loops, 0);
            MIX_PlayTrack(soundTrack, props);
            if (props) SDL_DestroyProperties(props);

            // Register the track as a "channel" so the static helpers below
            // (Stop/Pause/SetVolume/...) can find it by integer.
            auto it = std::find(g_channels.begin(), g_channels.end(), soundTrack);
            if (it == g_channels.end()) {
                g_channels.push_back(soundTrack);
                chan = (int)g_channels.size() - 1;
            } else {
                chan = (int)(it - g_channels.begin());
            }
        }
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

    if (realChannel >= 0 && soundTrack) {
        // FUTURE: the old Mix_SetPosition() did 3D spatialization. SDL3_mixer
        // exposes MIX_SetTrack3DPosition(track, &MIX_Point3D{x,y,z}). We approximate
        // the angle/distance as a point on a circle so the call is wired up;
        // a full 3D listener setup would go here too.
        float rad = (float)angle * (float)M_PI / 180.0f;
        float d   = (float)distance / 255.0f;          // normalize 0..1
        MIX_Point3D pos;
        pos.x = SDL_cosf(rad) * d;
        pos.y = 0.0f;
        pos.z = -SDL_sinf(rad) * d;
        MIX_SetTrack3DPosition(soundTrack, &pos);
    }

    return realChannel;
}


//
// Dispose
//
void SS_Sound::Dispose()
{
    // Remove our track from the channel registry.
    if (soundTrack) {
        auto it = std::find(g_channels.begin(), g_channels.end(), soundTrack);
        if (it != g_channels.end()) g_channels.erase(it);
        MIX_DestroyTrack(soundTrack);
        soundTrack = NULL;
    }
    if (soundChunk) {
        MIX_DestroyAudio(soundChunk);
        soundChunk = NULL;
    }
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
        // MIX_Init() must be called before creating a mixer.
        MIX_Init();

        // Build the mixer format. The old code used
        //   Mix_OpenAudio(22050, AUDIO_S16MSB, 2, 512)
        // AUDIO_S16SYS is the host-native 16-bit format (LE on Apple Silicon),
        // which is what SDL3 wants here.
        SDL_zero(g_spec);
        g_spec.freq = 22050;
        g_spec.format = AUDIO_S16SYS;
        g_spec.channels = 2;

        g_mixer = MIX_CreateMixer(&g_spec);
        if (!g_mixer)
            printf("Couldn't open audio: %s\n", SDL_GetError());

        // Old API: Mix_AllocateChannels(channels) reserved N channels up front.
        // In the new model tracks are created on demand, so nothing to allocate.
        // 'channels' is kept for API compatibility.
        (void)channels;

        mixerInit = true;
    }
}


//--------------------------------------------------------------
// Static channel helpers (old integer-channel API)
//--------------------------------------------------------------

static MIX_Track *track_for_channel(int channel)
{
    if (channel < 0 || channel >= (int)g_channels.size()) return NULL;
    return g_channels[channel];
}

void SS_Sound::MixHalt(int channel)
{
    MIX_Track *t = track_for_channel(channel);
    if (t) MIX_StopTrack(t, 0);   // FUTURE: accept fade-out ms via ms_to_frames()
}

void SS_Sound::MixPause(int channel)
{
    MIX_Track *t = track_for_channel(channel);
    if (t) MIX_PauseTrack(t);
}

void SS_Sound::MixResume(int channel)
{
    MIX_Track *t = track_for_channel(channel);
    if (t) MIX_ResumeTrack(t);
}

void SS_Sound::MixFadeOut(int channel, int ms)
{
    MIX_Track *t = track_for_channel(channel);
    if (t) MIX_StopTrack(t, ms_to_frames((Uint32)ms));
}

void SS_Sound::MixVolume(int channel, int vol)
{
    MIX_Track *t = track_for_channel(channel);
    if (t) MIX_SetTrackGain(t, (float)vol / 128.0f);   // old range 0..128 -> 0.0..1.0
}

void SS_Sound::MixPan(int channel, Uint8 pan)
{
    MIX_Track *t = track_for_channel(channel);
    if (t) {
        // Old Mix_SetPanning(channel, left, right); we called it as
        // Mix_SetPanning(channel, pan, 255 - pan).
        MIX_StereoGains g;
        g.left  = (float)pan / 255.0f;
        g.right = (float)(255 - pan) / 255.0f;
        MIX_SetTrackStereo(t, &g);
    }
}

bool SS_Sound::MixPlaying(int channel)
{
    MIX_Track *t = track_for_channel(channel);
    return (t && MIX_TrackPlaying(t));
}


#pragma mark -
//--------------------------------------------------------------
// SS_Music
//--------------------------------------------------------------

SS_Music::SS_Music()
{
    DEBUGF(1, "[%p] SS_Music::SS_Music()\n", this);

    Init();
}

SS_Music::SS_Music(char *filename)
{
    DEBUGF(1, "[%p] SS_Music::SS_Music(%s)\n", this, filename);

    Init();
    Load(filename);
}

SS_Music::~SS_Music()
{
    DEBUGF(1, "[%p] SS_Music::~SS_Music()\n", this);

    Dispose();
}


//
// Init
//
void SS_Music::Init()
{
    DEBUGF(1, "[%p] SS_Music::Init()\n", this);

    flags = 0;
    music = NULL;
    musicTrack = NULL;
}


//
// Load(filename)
//
void SS_Music::Load(char *filename)
{
    DEBUGF(1, "[%p] SS_Music::Load(%s)\n", this, filename);
    char        *full = SS_Folder::FullPath(filename);

    // Same MIX_Audio type as SFX; just a longer sample. (Streaming via
    // MIX_SetTrackIOStream would be the FUTURE choice for very large tracks.)
    if ( ! (music = MIX_LoadAudio(g_mixer, full, true)) ) {
        printf("%s\n", SDL_GetError());
        throw ("Can't load music file!");
    }
}


//
// MusicPlay helper: ensure a track exists, set the audio + loops, play.
// fadeMs > 0 gives a fade-in (old Mix_FadeInMusic).
//
int SS_Music::MusicPlay(MIX_Track *&track, MIX_Audio *audio, int loops, int fadeMs)
{
    if (!track) {
        track = MIX_CreateTrack(g_mixer);
        if (!track) return 0;
    }
    MIX_SetTrackAudio(track, audio);
    MIX_SetTrackLoops(track, loops);
    SDL_PropertiesID props = make_play_props(loops, fadeMs);
    bool ok = MIX_PlayTrack(track, props);
    if (props) SDL_DestroyProperties(props);
    return ok ? 1 : 0;
}


//
// FadeOut — stop with a short fade (old Mix_FadeOutMusic took ms).
//
void SS_Music::FadeOut(int ms)
{
    if (musicTrack)
        MIX_StopTrack(musicTrack, ms_to_frames((Uint32)ms));
}


//
// Pause / Stop
//
void SS_Music::Pause()
{
    if (musicTrack) MIX_PauseTrack(musicTrack);
}

void SS_Music::Stop()
{
    if (musicTrack) MIX_StopTrack(musicTrack, 0);
}


//
// SetVolume — old range 0..128
//
void SS_Music::SetVolume(Uint32 vol)
{
    if (musicTrack) MIX_SetTrackGain(musicTrack, (float)vol / 128.0f);
}


//
// SetRate / SetFrequency — speed/pitch via frequency ratio.
// FUTURE: a true absolute-rate API would divide by the source sample rate.
//
void SS_Music::SetRate(Uint32 rate)
{
    if (musicTrack && g_spec.freq)
        MIX_SetTrackFrequencyRatio(musicTrack, (float)rate / (float)g_spec.freq);
}

void SS_Music::SetFrequency(int freq)
{
    if (musicTrack && g_spec.freq)
        MIX_SetTrackFrequencyRatio(musicTrack, (float)freq / (float)g_spec.freq);
}
