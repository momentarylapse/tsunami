/*
 * HuiApplication.cpp
 *
 *  Created on: 13.07.2014
 *      Author: michi
 */

#include "Application.h"

#include "hui.h"
#include "internal.h"

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
bool Application::running;

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
	if (Config.changed)
		Config.save(directory << "config.txt");
	if ((msg_inited) /*&& (HuiMainLevel == 0)*/)
		msg_end();
}

Path strip_dev_dirs(const Path &p) {
	if (p.basename() == "build")
		return p.parent();
	if (p.basename() == "Debug")
		return p.parent();
	if (p.basename() == "Release")
		return p.parent();
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
	hui_win_instance = (HINSTANCE)GetModuleHandle(nullptr);
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

// deprecated...
void Application::_init(const Array<string> &arg, const string &program, bool load_res, const string &def_lang) {
}

int Application::run() {
	running = true;
#ifdef HUI_API_WIN
	MSG messages;
	messages.message = 0;
	bool HuiHaveToExit = false;
	bool got_message;
	while ((!HuiHaveToExit) and (WM_QUIT!=messages.message)){
		bool allow=true;
		if (_idle_function_.is_set())
			got_message=(PeekMessage(&messages,nullptr,0U,0U,PM_REMOVE)!=0);
		else
			got_message=(GetMessage(&messages,nullptr,0,0)!=0);
		if (got_message){
			allow=false;
			TranslateMessage(&messages);
			DispatchMessage(&messages);
			for (int i=0;i<_hui_windows_.num;i++)
				if (_hui_windows_[i]->hWnd == messages.hwnd){
					allow=true;
					break;
				}
		}
		if (_idle_function_.is_set() and allow)
			_idle_function_.call();
	}
#endif
#ifdef HUI_API_GTK
	gtk_main();
#endif
	return 0;
}

// FIXME: when closing the last window, hard_end() gets called... ignoring onEnd()!!!
void Application::end() {
	SetIdleFunction(nullptr);

	on_end();

	hard_end();
}

// ends the system loop of the run() command
void Application::hard_end() {
	SetIdleFunction(nullptr);

	foreachb(Window *w, _all_windows_)
		delete(w);

	// send "quit" message
#ifdef HUI_API_WIN
	PostQuitMessage(0);
#endif
#ifdef HUI_API_GTK
	if (gtk_main_level() > 0)
		gtk_main_quit();
#endif

	// really end hui?
#ifdef HUI_API_GTK
#ifdef OS_LINUX
	// sometimes freezes...
	//if (hui_x_display)
	//	XCloseDisplay(hui_x_display);
#endif

#if GTK_CHECK_VERSION(3,0,0)
	g_object_unref(invisible_cursor);
#endif
#endif
	if (Config.changed)
		Config.save(directory << "config.txt");
	if (msg_inited)
		msg_end();
}
void Application::do_single_main_loop() {
#ifdef HUI_API_WIN
	MSG messages;
	messages.message=0;
	HuiHaveToExit=false;
	bool got_message;

	bool allow=true;
	if (_idle_function_)
		got_message=(PeekMessage(&messages,nullptr,0U,0U,PM_REMOVE)!=0);
	else
		got_message=(GetMessage(&messages,nullptr,0,0)!=0);
	if (got_message){
		allow=false;
		TranslateMessage(&messages);
		DispatchMessage(&messages);
		for (int i=0;i<_hui_windows_.num;i++)
			if (_hui_windows_[i]->hWnd == messages.hwnd){
				allow = true;
				return;
			}
	}
	/*if (HuiIdleFunction and allow)
		HuiIdleFunction();*/
#endif
#ifdef HUI_API_GTK

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
#endif
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
