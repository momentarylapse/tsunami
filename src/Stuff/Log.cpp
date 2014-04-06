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


void Log::Error(const string &message)
{
	AddMessage(TYPE_ERROR, message);
}


void Log::Warning(const string &message)
{
	AddMessage(TYPE_WARNING, message);
}


void Log::Info(const string &message)
{
	AddMessage(TYPE_INFO, message);
}


void Log::Clear()
{
	messages.clear();
	Notify(MESSAGE_CLEAR);
}


void Log::AddMessage(int type, const string &_message)
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
	Notify(MESSAGE_ADD);
}
