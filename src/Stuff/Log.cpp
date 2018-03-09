/*
 * Log.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Log.h"
#include "../lib/hui/hui.h"
#include "../Session.h"

const string Log::MESSAGE_ADD = "Add";


void Log::error(Session *session, const string &message)
{
	addMessage(session, TYPE_ERROR, message);
}


void Log::warn(Session *session, const string &message)
{
	addMessage(session, TYPE_WARNING, message);
}


void Log::info(Session *session, const string &message)
{
	addMessage(session, TYPE_INFO, message);
}


Log::Message Log::last()
{
	return messages.back();
}

Array<Log::Message> Log::all(Session *session)
{
	Array<Log::Message> r;
	for (auto &m: messages)
		if ((m.session == session) or (m.session == Session::GLOBAL))
			r.add(m);
	return r;
}


void Log::addMessage(Session *session, int type, const string &_message)
{
	Message m;
	m.session = session;
	m.type = type;
	m.text = _message;
	messages.add(m);

	if (type == TYPE_ERROR){
		msg_error(_message);
	}else if (type == TYPE_WARNING){
		msg_write(_message);
	}else{
		msg_write(_message);
	}
	notify(MESSAGE_ADD);
}
