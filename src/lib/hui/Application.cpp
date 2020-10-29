/*
 * HuiApplication.cpp
 *
 *  Created on: 13.07.2014
 *      Author: michi
 */

#include "Application.h"

#include "hui.h"
#include "internal.h"

#ifdef OS_WINDOWS
#include <windows.h>
#endif

namespace hui
{

extern Callback _idle_function_;


#ifdef HUI_API_GTK
	extern void *invisible_cursor;
#endif


Map<string, string> Application::_properties_;

Path Application::filename;
Path Application::directory;
Path Application::directory_static;
Path Application::initial_working_directory;
bool Application::installed;

Array<string> Application::_args;



Application::Application(const string &app_name, const string &def_lang, int flags) {

#ifdef HUI_API_GTK
	g_set_prgname(app_name.c_str());
#endif

	guess_directories(_args, app_name);


	_InitInput_();

	ComboBoxSeparator = "\\";
	_using_language_ = false;
	SetDefaultErrorHandler(nullptr);

	if (file_exists(directory << "config.txt"))
		Config.load(directory << "config.txt");


	if (flags & FLAG_LOAD_RESOURCE)
		LoadResource(directory_static << "hui_resources.txt");

	if (def_lang.num > 0)
		SetLanguage(Config.get_str("Language", def_lang));


	if (file_exists(directory_static << "icon.svg"))
		set_property("logo", (directory_static << "icon.svg").str());
	else if (file_exists(directory_static << "icon.png"))
		set_property("logo", (directory_static << "icon.png").str());
	else if (file_exists(directory_static << "icon.ico"))
		set_property("logo", (directory_static << "icon.ico").str());
}

Application::~Application() {
	//foreachb(Window *w, _all_windows_)
	//	delete(w);
	if (Config.changed)
		Config.save(directory << "config.txt");
	if ((msg_inited) /*&& (HuiMainLevel == 0)*/)
		msg_end();
}

Path strip_dev_dirs(const Path &p) {
	if (p.basename() == "build")
		return p.parent();
	if (p.basename() == "Debug")
		return strip_dev_dirs(p.parent());
	if (p.basename() == "Release")
		return strip_dev_dirs(p.parent());
	if (p.basename() == "x64")
		return p.parent();
	if (p.basename() == "Unoptimized")
		return p.parent();
	return p;
}

//   filename -> executable file
//   directory ->
//      NONINSTALLED:  binary dir
//      INSTALLED:     ~/.MY_APP/
//   directory_static ->
//      NONINSTALLED:  binary dir/static/
//      INSTALLED:     /usr/local/share/MY_APP/
//   initial_working_directory -> working dir before running this program
void Application::guess_directories(const Array<string> &arg, const string &app_name) {

	initial_working_directory = get_current_dir();
	installed = false;


	// executable file
#if defined(OS_LINUX) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
	if (arg.num > 0)
		filename = arg[0];
#else // OS_WINDOWS
	char *ttt = nullptr;
	int r = _get_pgmptr(&ttt);
	filename = ttt;
	hui_win_instance = (void*)GetModuleHandle(nullptr);
#endif


	// first, assume a local/non-installed version
	directory = strip_dev_dirs(filename.parent());
	directory_static = directory << "static";


	#if defined(OS_LINUX) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
		// installed version?
		if (filename.is_in("/usr/local") or (filename.str().find("/") < 0)) {
			installed = true;
			directory_static = Path("/usr/local/share") << app_name;
		} else if (filename.is_in("/usr")) {
			installed = true;
			directory_static = Path("/usr/share") << app_name;
		} else if (filename.is_in("/opt")) {
			installed = true;
			directory_static = Path("/opt") << app_name;
		//} else if (f) {
		}

		if (installed) {
			directory = format("%s/.%s/", getenv("HOME"), app_name);
			dir_create(directory);
		}
	#endif
}

int Application::run() {
	gtk_main();

	on_end();
	return 0;
}

void Application::end() {
	SetIdleFunction(nullptr);
	gtk_main_quit();
}

void Application::do_single_main_loop() {
	// push idle function
	//Callback _if_ = _idle_function_;

	//SetIdleFunction(NULL);
	int counter = 0;
	do {
		g_main_context_iteration(nullptr, false);
		gtk_main_iteration_do(false);
		counter ++;
		if (counter > 5)
			break;
	} while (gtk_events_pending());

	// pop idle function
	//SetIdleFunction(_if_);
}


void Application::set_property(const string &name, const string &value) {
	_properties_.set(name, value);
}

string Application::get_property(const string &name) {
	try {
		return _properties_[name];
	} catch(...) {
		return "";
	}
}

void Application::about_box(Window *win) {
	AboutBox(win);
}

};
