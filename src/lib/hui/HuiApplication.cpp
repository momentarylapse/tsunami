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
	if (app->onStartup(arg))
		return app->run();
	delete(app);
	return 0;
}

