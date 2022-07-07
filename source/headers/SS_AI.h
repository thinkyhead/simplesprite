/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_AI.h
 *
 *  $Id: SS_AI.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 *  Derived from the book
 *  Data Structures for Game Programmers
 *  by Ron Penton
 *  ISBN 1-931841-94-2
 *
 */

#ifndef __SS_AI_H__
#define __SS_AI_H__

enum AIType {
    AI_ATTACKER,
    AI_DEFENDER,
    AI_HUNTER,
    AI_COLLECTOR,
    AI_MERCHANT
};

enum AIState {
    AI_MAINGOAL,
    AI_ATTACKING,
    AI_FINDHEALTH,
    AI_DOCKING,
    AI_NUM_STATES
};

enum AIEvent {
    AI_SEEPLAYER,
    AI_KILLENEMY,
    AI_GOTHEALTH,
    AI_DOCKED,
    AI_NUM_EVENTS
};

enum HealthState {
    AI_GOODHEALTH,
    AI_BADHEALTH,
    AI_NUM_HEALTHS
};

class SS_AI
{
    private:
        AIState m_state;
        AIType  m_type;
        float   m_x, m_y;
        int     m_health;

    public:
        SS_AI();
        ~SS_AI();

        void Init(AIType t, float x, float y, int h)
        {
            m_type      = t;
            m_x         = x;
            m_y         = y;
            m_health    = h;
            m_state     = AI_MAINGOAL;
        }
};


#endif