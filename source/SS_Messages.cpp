/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Messages.cpp
 *
 *  $Id: SS_Messages.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Messages.h"

//--------------------------------------------------------------
// SS_Listener
//

SS_Listener::SS_Listener()
{
    DEBUGF(1, "[%08X] SS_Listener() CONSTRUCTOR\n", this);
}

SS_Listener::~SS_Listener()
{
    DEBUGF(1, "[%08X] SS_Listener() DESTRUCTOR\n", this);

    IgnoreAll();
}

//
// ListenTo(sender)
//
void SS_Listener::ListenTo(SS_Broadcaster *sender)
{
    DEBUGF(1, "[%08X] SS_Listener::ListenTo(%08X)\n", this, sender);

    sender->AddListener(this);
}

//
// Ignore(sender)
//
void SS_Listener::Ignore(SS_Broadcaster *sender)
{
    DEBUGF(1, "[%08X] SS_Listener::Ignore(%08X)\n", this, sender);

    sender->RemoveListener(this);
    RemoveBroadcaster(sender);
}


//
// IgnoreAll()
//
void SS_Listener::IgnoreAll()
{
    DEBUGF(1, "[%08X] SS_Listener::IgnoreAll()\n", this);

    SS_Broadcaster *item;
    SS_BroadcasterIterator itr = broadcasterList.GetIterator();
    while (item = itr.NextItem())
        item->RemoveListener(this);

    broadcasterList.Clear();
}


#pragma mark -
//--------------------------------------------------------------
// SS_Broadcaster
//

SS_Broadcaster::SS_Broadcaster()
{
    DEBUGF(1, "[%08X] SS_Broadcaster() CONSTRUCTOR\n", this);

    listenerIterator = listenerList.GetIterator();
}

SS_Broadcaster::~SS_Broadcaster()
{
    SS_Listener *listener;
    listenerIterator.Start();
    while (listener = listenerIterator.NextItem())
        listener->RemoveBroadcaster(this);
}

//
// Broadcast(message)
//
void SS_Broadcaster::Broadcast(const SS_Message &msg)
{
    SS_Listener *listener;
    listenerIterator.Start();
    while (listener = listenerIterator.NextItem())
        listener->HandleMessage(msg);
}


