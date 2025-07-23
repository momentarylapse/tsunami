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
#include "../os/path.h"
#include "common.h"

typedef struct _GtkApplication GtkApplication;

namespace hui {

class Window;

enum class Flags {
	NONE = 0,
	DONT_LOAD_RESOURCE = 1,
	SILENT = 2,
	NO_ERROR_HANDLER = 4,
	UNIQUE = 16,
};
Flags operator|(Flags a, Flags b);
int operator&(Flags a, Flags b);

enum class AppStatus {
	END,
	RUN,
	AUTO
};

class Application : public VirtualBase {
public:
	Application(const string &app_name, const string &def_lang, Flags flags);
	~Application() override;

	virtual AppStatus on_startup_before_gui_init(const Array<string> &arg);
	void run_after_gui_init(std::function<void()> f);

	virtual AppStatus on_startup(const Array<string> &arg) = 0;
	virtual void on_end() {}

	static void end();

	int run();
	static void do_single_main_loop();



	static void _cdecl about_box(Window *win);
	static void _cdecl set_property(const string &name, const string &value);
	static string get_property(const string &name);

	static base::map<string, string> _properties_;

	static bool adwaita_started;
	static Flags flags;
	static bool allowed;

	static Array<string> _args;

	static GtkApplication *application;
	int try_execute(const Array<string> &args);
};

}

#define HUI_EXECUTE(APP_CLASS) \
namespace os::app { \
int main(const Array<string> &args) { \
	APP_CLASS::_args = args; \
	APP_CLASS *app = new APP_CLASS; \
	int r = app->try_execute(args); \
	delete app; \
	return r; \
} \
}

#endif /* HUIAPPLICATION_H_ */
