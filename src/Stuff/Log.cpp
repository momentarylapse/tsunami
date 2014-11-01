/*
 * Log.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Log.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"

const string Log::MESSAGE_ADD = "Add";
const string Log::MESSAGE_CLEAR = "Clear";

Log::Log() :
	Observable("Log")
{
}

Log::~Log()
{
}


void Log::error(const string &message)
{
	addMessage(TYPE_ERROR, message);
}


void Log::warning(const string &message)
{
	addMessage(TYPE_WARNING, message);
}


void Log::info(const string &message)
{
	addMessage(TYPE_INFO, message);
}


void Log::clear()
{
	messages.clear();
	notify(MESSAGE_CLEAR);
}


void Log::addMessage(int type, const string &_message)
{
	Message m;
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
