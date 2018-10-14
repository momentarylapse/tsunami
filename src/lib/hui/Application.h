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

	virtual bool on_startup(const Array<string> &arg) = 0;
	virtual void on_end(){}

	void end();

	static int run();
	static void hard_end();
	static void do_single_main_loop();




	static void _cdecl about_box(Window *win);
	static void _cdecl set_property(const string &name, const string &value);
	static string get_property(const string &name);

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
	if (app->on_startup(arg)) \
		r = app->run(); \
	delete(app); \
	return r; \
}

#endif /* HUIAPPLICATION_H_ */
