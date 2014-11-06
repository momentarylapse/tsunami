/*
 * HuiApplication.cpp
 *
 *  Created on: 13.07.2014
 *      Author: michi
 */

#include "HuiApplication.h"
#include "hui.h"

HuiApplication::HuiApplication(const string &app_name, const string &def_lang, int flags)
{
	if (flags & HUI_FLAG_SILENT)
		msg_init(true);
	HuiInit(app_name, (flags & HUI_FLAG_LOAD_RESOURCE), def_lang);

	if (flags & HUI_FLAG_SILENT)
		msg_init(HuiAppDirectory + "message.txt", false);

	HuiEndKeepMsgAlive = true;
}

HuiApplication::~HuiApplication()
{
}

int HuiApplication::run()
{
	return HuiRun();
}

int HuiApplication::_Execute_(HuiApplication *app, const Array<string> &arg)
{
	int r = 0;
	if (app->onStartup(arg))
		r = app->run();
	delete(app);
	if (HuiConfig.changed)
		HuiConfig.save();

	if ((msg_inited) /*&& (HuiMainLevel == 0)*/)
		msg_end();
	return r;
}

