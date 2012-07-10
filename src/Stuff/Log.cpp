/*
 * Log.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Log.h"
#include "../lib/hui/hui.h"
#include "../Tsunami.h"

Log::Log(CHuiWindow *parent)
{
	dlg = HuiCreateSizableDialog(_("Meldungen"), 500, 300, parent, true);
	dlg->ToolbarSetByID("log_toolbar");
	dlg->AddListView("!nobar,format=it\\type\\msg", 0, 0, 0, 0, "log_list");
	dlg->Hide(true);
	dlg->Update();

	dlg->EventM("hui:close", this, (void(HuiEventHandler::*)())&Log::Close);
	dlg->EventM("log_close", this, (void(HuiEventHandler::*)())&Log::Close);
	dlg->EventM("log_clear", this, (void(HuiEventHandler::*)())&Log::Clear);
}

Log::~Log()
{
	delete(dlg);
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
	dlg->Reset("log_list");
}


void Log::Close()
{
	dlg->Hide(true);
}


void Log::Show()
{
	dlg->Hide(false);
}


void Log::AddMessage(int type, const string &_message)
{
	Message m;
	m.type = type;
	m.text = _message;
	message.add(m);

	if (type == TYPE_ERROR){
		msg_error(_message);
		dlg->AddString("log_list", "hui:error\\" + _message);
	}else if (type == TYPE_WARNING){
		msg_write(_message);
		dlg->AddString("log_list", "hui:warning\\" + _message);
	}else{
		msg_write(_message);
		dlg->AddString("log_list", "hui:info\\" + _message);
	}

	if ((type == TYPE_ERROR) || (type == TYPE_WARNING))
		Show();
		//HuiErrorBox(HuiCurWindow, _("Fehler"), _message);
}
