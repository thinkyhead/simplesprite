/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Config.h
 *
 *  $Id: SS_Config.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 *  Edit this file to configure the game's global environment
 *
 */

#ifndef __SS_CONFIG_H__
#define __SS_CONFIG_H__

                        //
                        // Debugger output? (0-2)
                        //
#define SS_DEBUG            0

                        //
                        // Threaded? (1 / 0)
                        //
#define SS_THREADS          0

                        //
                        // Using audio I hope?
                        //
#define SS_AUDIO_ENABLE     1

                        //
                        // Joystick input is appreciated by some
                        //
#define SS_JOYSTICK_ENABLE  1

                        //
                        // Maximum number of concurrent worlds
                        //
#define SS_MAX_WORLDS       10

                        //
                        // Collision List arrangement
                        //
#define SS_COLLISION_LISTS      100
#define SS_COLLISION_BAND_SIZE  200

                        //
                        // Debug output macro (also used by SS_Templates.h)
                        //
#define DEBUGF(N,V...) if (SS_DEBUG >= N) printf(V)

                        //
                        // Asserts for testing (gated by SS_ASSERT_ON, set by the
                        // Debug CMake config). Fires on ref-count underflow, NaN
                        // angles, and dangling pointers — the usual SDL2-port gotchas.
                        //
#ifdef SS_ASSERT_ON
  #include <cassert>
  #include <cmath>
  #define SS_ASSERT(c)        assert(c)
  #define SS_ASSERT_FINITE(f) assert(std::isfinite(f))
#else
  #define SS_ASSERT(c)        ((void)0)
  #define SS_ASSERT_FINITE(f) ((void)0)
#endif

#endif
