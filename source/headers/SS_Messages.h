/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Messages.h
 *
 *  $Id: SS_Messages.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_MESSAGES_H__
#define __SS_MESSAGES_H__

#include <SDL.h>
#include "SS_Templates.h"

//--------------------------------------------------------------
// Forward declarations
//
class SS_Broadcaster;
typedef TListNode<SS_Broadcaster*>		SS_BroadcasterNode;
typedef TObjectList<SS_Broadcaster*>	SS_BroadcasterList;
typedef TIterator<SS_Broadcaster*>		SS_BroadcasterIterator;


//--------------------------------------------------------------
// SS_Message
//
class SS_Message
{
	private:
		Uint32  message_class;
		Uint32  message_code;
		int		message_value;
		const void	*message_data;

	public:
		SS_Message() { Set(0,0); }
		SS_Message(Uint32 mclass, Uint32 mcode, int mval=0, void *data=NULL) { Set(mclass, mcode, mval, data); }

		inline Uint32	Class() const	{ return message_class; }
		inline Uint32	Code() const	{ return message_code; }
		inline int		Value() const	{ return message_value; }
		inline const void*	Data() const	{ return message_data; }
		inline bool		IsSet() const	{ return Class() && Code(); }

		inline void		Set(Uint32 mclass, Uint32 mcode, int mval=0, void *data=NULL)
				{
					message_class = mclass;	message_code = mcode;
					message_value = mval;	message_data = data;
				}
		inline void		SetClass(Uint32 mclass)	{ message_class = mclass; }
		inline void		SetCode(Uint32 mcode)	{ message_code = mcode; }
		inline void		SetValue(int mval)		{ message_value = mval; }
		inline void		SetData(void *mdata)	{ message_data = mdata; }

		inline float	GetFloat() const	{ return message_data ? *((float*)message_data) : 0.0f; }
		inline Sint16	GetInt16() const	{ return message_data ? *((Sint16*)message_data) : 0; }
		inline Sint32	GetInt32() const	{ return message_data ? *((Sint32*)message_data) : 0; }
		inline char*	GetString() const	{ return (char*)message_data; }
};


#pragma mark -
//--------------------------------------------------------------
// SS_Listener
// Receives messages sent by SS_Broadcaster
//--------------------------------------------------------------
class SS_Listener
{
	private:
		SS_BroadcasterList		broadcasterList;

	public:
						SS_Listener();
		virtual			~SS_Listener();

		void			ListenTo(SS_Broadcaster *sender);
		void			Ignore(SS_Broadcaster *sender);
		void			IgnoreAll();

		inline void		RemoveBroadcaster(SS_Broadcaster *b)	{ broadcasterList.Remove(b, false); }
		inline void		RemoveAllBroadcasters()					{ broadcasterList.Clear(); }

		virtual void	HandleMessage(const SS_Message &message) {}
		inline void		HandleMessage(Uint32 mclass, Uint32 mcode, int mval=0, void *mdata=NULL) { HandleMessage(SS_Message(mclass, mcode, mval, mdata)); }
};

typedef TListNode<SS_Listener*>		SS_ListenerNode;
typedef TObjectList<SS_Listener*>	SS_ListenerList;
typedef TIterator<SS_Listener*>		SS_ListenerIterator;


#pragma mark -
//--------------------------------------------------------------
// SS_Broadcaster
// Sends messages to one or more listeners
//--------------------------------------------------------------
class SS_Broadcaster
{
	friend class SS_Listener;

	private:
		SS_ListenerList		listenerList;
		SS_ListenerIterator listenerIterator;

	public:
		SS_Message			message;

						SS_Broadcaster();
						~SS_Broadcaster();

		inline void		SetMessage(Uint32 mclass, Uint32 mcode, int mval=0, void *mdata=NULL) { message.Set(mclass, mcode, mval, mdata); }
		inline bool		MessageIsSet() const					{ return message.IsSet(); }
		inline void		AddListener(SS_Listener *listener)		{ listenerList.Append(listener); }
		inline void		RemoveListener(SS_Listener *listener)	{ listenerList.Remove(listener, false); }
		inline void		RemoveAllListeners()					{ listenerList.Clear(); }

	protected:
		void			Broadcast() { Broadcast(message); }
		void			Broadcast(const SS_Message &msg);
		inline void		Broadcast(Uint32 mclass, Uint32 mcode, int mval=0, void *data=NULL) { Broadcast(SS_Message(mclass, mcode, mval, data)); }
		inline void		Broadcast(char *text) { message.SetData(text); Broadcast(); }
		inline void		Broadcast(float &num) { message.SetData(&num); Broadcast(); }
		inline void		Broadcast(int &num)  { message.SetData(&num); Broadcast(); }
};


#endif
