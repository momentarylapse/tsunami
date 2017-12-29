/*
 * HuiApplication.h
 *
 *  Created on: 13.07.2014
 *      Author: michi
 */

#ifndef HUIAPPLICATION_H_
#define HUIAPPLICATION_H_

#include "../base/base.h"
#include "../base/map.h"
#include "common.h"

namespace hui
{

class Window;

enum
{
	FLAG_LOAD_RESOURCE = 1,
	FLAG_SILENT = 2,
	FLAG_UNIQUE = 16,
};

class Application : public VirtualBase
{
public:
	Application(const string &app_name, const string &def_lang, int flags);
	virtual ~Application();

	static void _init(const Array<string> &arg, const string &program, bool load_res, const string &def_lang);

	virtual bool onStartup(const Array<string> &arg) = 0;

	static int run();
	static void end();
	static void doSingleMainLoop();




	static void _cdecl aboutBox(Window *win);
	static void _cdecl setProperty(const string &name, const string &value);
	static string getProperty(const string &name);

	static Map<string, string> _properties_;

	static string filename;
	static string directory;			// dir of changeable files (ie. ~/.app/)
	static string directory_static;	// dir of static files (ie. /usr/shar/app)
	static string initial_working_directory;
	static bool installed; // installed into system folders?
	static bool running;

	static Array<string> _args;
};

}

#define HUI_EXECUTE(APP_CLASS) \
int hui_main(const Array<string> &arg) \
{ \
	APP_CLASS::_args = arg; \
	APP_CLASS *app = new APP_CLASS; \
	int r = 0; \
	if (app->onStartup(arg)) \
		r = app->run(); \
	delete(app); \
	return r; \
}

#endif /* HUIAPPLICATION_H_ */
