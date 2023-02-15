/*
 * HuiApplication.cpp
 *
 *  Created on: 13.07.2014
 *      Author: michi
 */

#include "Application.h"

#include "hui.h"
#include "internal.h"
#include "../os/filesystem.h"

#ifdef OS_WINDOWS
#include <windows.h>
#endif

namespace hui
{

extern Callback _idle_function_;


#ifdef HUI_API_GTK
	extern void *invisible_cursor;
#endif

GtkApplication *Application::application = nullptr;


base::map<string, string> Application::_properties_;

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
	if ((flags & FLAG_NO_ERROR_HANDLER) == 0)
		SetDefaultErrorHandler(nullptr);

	if (os::fs::exists(directory | "config.txt"))
		config.load(directory | "config.txt");


	if ((flags & FLAG_DONT_LOAD_RESOURCE) == 0)
		load_resource(directory_static | "hui_resources.txt");

	if (def_lang.num > 0)
		set_language(config.get_str("Language", def_lang));


#ifdef OS_LINUX
	if (os::fs::exists(directory_static | "icon.svg"))
		set_property("logo", (directory_static | "icon.svg").str());
	else
#endif
	if (os::fs::exists(directory_static | "icon.png"))
		set_property("logo", (directory_static | "icon.png").str());
	else if (os::fs::exists(directory_static | "icon.ico"))
		set_property("logo", (directory_static | "icon.ico").str());


#if GTK_CHECK_VERSION(4,0,0)
	application = gtk_application_new(nullptr, G_APPLICATION_NON_UNIQUE);
#endif
}

Application::~Application() {
	//foreachb(Window *w, _all_windows_)
	//	delete(w);
	if (config.changed)
		config.save(directory | "config.txt");
	if ((msg_inited) /*&& (HuiMainLevel == 0)*/)
		msg_end();

#if GTK_CHECK_VERSION(4,0,0)
	g_object_unref(application);
#endif
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
//      INSTALLED:     ~/.MY_APP/      <<< now always this
//   directory_static ->
//      NONINSTALLED:  binary dir/static/
//      INSTALLED:     /usr/local/share/MY_APP/
//   initial_working_directory -> working dir before running this program
void Application::guess_directories(const Array<string> &arg, const string &app_name) {

	initial_working_directory = os::fs::current_directory();
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
	directory = initial_working_directory; //strip_dev_dirs(filename.parent());
	directory_static = directory | "static";

#ifdef INSTALL_PREFIX
	// our build system should define this:
	Path prefix = INSTALL_PREFIX;
#else
	// oh no... fall-back
	Path prefix = "/usr/local";
#endif

	#if defined(OS_LINUX) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
		// installed version?
		if (filename.is_in(prefix) or (filename.str().find("/") < 0)) {
			installed = true;
			directory_static = prefix | "share" | app_name;
		}

		directory = format("%s/.%s/", getenv("HOME"), app_name);
		os::fs::create_directory(directory);
	#endif
}

#if GTK_CHECK_VERSION(4,0,0)
static bool keep_running = true;
#endif

int Application::run() {
#if GTK_CHECK_VERSION(4,0,0)
	return g_application_run(G_APPLICATION (application), 0, nullptr);
	while (keep_running)
		do_single_main_loop();
#else
	gtk_main();
#endif

	on_end();
	return 0;
}


[[maybe_unused]] static void on_gtk_application_activate(GApplication *_g_app, gpointer user_data) {
	auto app = reinterpret_cast<Application*>(user_data);
	app->on_startup(app->_args);
}



int Application::try_execute(const Array<string> &args) {
#if GTK_CHECK_VERSION(4,0,0)
	g_signal_connect(application, "activate", G_CALLBACK(on_gtk_application_activate), this);
	return g_application_run(G_APPLICATION(application), 0, nullptr);
#endif
	if (on_startup(args))
		return run();
	return 0;
}

void Application::end() {
	set_idle_function(nullptr);
#if GTK_CHECK_VERSION(4,0,0)
	keep_running = false;
#else
	gtk_main_quit();
#endif
}

void Application::do_single_main_loop() {
	// push idle function
	//Callback _if_ = _idle_function_;

	//SetIdleFunction(NULL);
	int counter = 0;
	do {
		g_main_context_iteration(nullptr, false);
#if !GTK_CHECK_VERSION(4,0,0)
		gtk_main_iteration_do(false);
#endif
		counter ++;
		if (counter > 5)
			break;
#if GTK_CHECK_VERSION(4,0,0)
	} while (g_main_context_pending(nullptr));
#else
	} while (gtk_events_pending());
#endif


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
	hui::about_box(win);
}

};
