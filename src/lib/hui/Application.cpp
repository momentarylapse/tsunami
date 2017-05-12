/*
 * HuiApplication.cpp
 *
 *  Created on: 13.07.2014
 *      Author: michi
 */

#include "Application.h"

#include "hui.h"

namespace hui
{

Application::Application(const string &app_name, const string &def_lang, int flags)
{
	if (flags & FLAG_SILENT)
		msg_init(true);
	Init(app_name, (flags & FLAG_LOAD_RESOURCE), def_lang);

	if (flags & FLAG_SILENT)
		msg_init(AppDirectory + "message.txt", false);

	EndKeepMsgAlive = true;
}

Application::~Application()
{
	if (Config.changed)
		Config.save();
	if ((msg_inited) /*&& (HuiMainLevel == 0)*/)
		msg_end();
}

int Application::run()
{
	return Run();
}

};
