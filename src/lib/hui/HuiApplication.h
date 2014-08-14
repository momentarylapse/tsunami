/*
 * HuiApplication.h
 *
 *  Created on: 13.07.2014
 *      Author: michi
 */

#ifndef HUIAPPLICATION_H_
#define HUIAPPLICATION_H_

#include "../base/base.h"
#include "hui_common.h"

enum
{
	HUI_FLAG_LOAD_RESOURCE = 1,
	HUI_FLAG_SILENT = 2,
	HUI_FLAG_UNIQUE = 16,
};

class HuiApplication : public HuiEventHandler
{
public:
	HuiApplication(Array<string> arg, const string &app_name, const string &def_lang, int flags);
	virtual ~HuiApplication();

	virtual bool onStartup(Array<string> arg) = 0;

	int run();
};

#define HuiExecute(APP_CLASS) \
int hui_main(Array<string> arg) \
{ \
	HuiApplication *app = new APP_CLASS(arg); \
	if (app->onStartup(arg)); \
		return app->run(); \
	delete(app); \
	return 0; \
}

#endif /* HUIAPPLICATION_H_ */
