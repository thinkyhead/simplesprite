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

#endif
