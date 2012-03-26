/*
 * Log.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Log.h"
#include "../lib/hui/hui.h"

Log::Log()
{
	// TODO Auto-generated constructor stub

}

Log::~Log()
{
	// TODO Auto-generated destructor stub
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
	message.clear();
}


void Log::AddMessage(int type, const string &_message)
{
	Message m;
	m.type = type;
	m.text = _message;
	message.add(m);

	msg_error(_message);
	if (type == TYPE_ERROR)
		HuiErrorBox(HuiCurWindow, _("Fehler"), _message);
}
