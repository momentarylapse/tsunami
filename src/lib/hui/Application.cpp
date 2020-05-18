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

string Application::filename;
string Application::directory;
string Application::directory_static;
string Application::initial_working_directory;
bool Application::installed;
bool Application::running;

Array<string> Application::_args;



Application::Application(const string &app_name, const string &def_lang, int flags) {
	initial_working_directory = get_current_dir();
	installed = false;

	#ifdef HUI_API_GTK
		g_set_prgname(app_name.c_str());
	#endif

	#if defined(OS_LINUX) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
		directory = initial_working_directory;
		directory_static = directory + "static/";
		if (_args.num > 0) {
			filename = _args[0].replace("\\", "/");
			directory = filename.dirname();


			if ((filename.head(11) == "/usr/local/") or (filename.find("/") < 0)) {
				installed = true;
				// installed version?
				directory = format("%s/.%s/", getenv("HOME"), app_name.c_str());
				directory_static = "/usr/local/share/" + app_name + "/";
			} else if ((filename.head(5) == "/usr/") or (filename.find("/") < 0)) {
				installed = true;
				// installed version?
				directory = format("%s/.%s/", getenv("HOME"), app_name.c_str());
				directory_static = "/usr/share/" + app_name + "/";
			}
		}
		dir_create(directory);
	#else // OS_WINDOWS
		char *ttt = nullptr;
		int r = _get_pgmptr(&ttt);
		filename = ttt;
		directory = filename.dirname();
		directory = directory.replace("\\Release\\", "\\");
		directory = directory.replace("\\Debug\\", "\\");
		directory = directory.replace("\\Unoptimized\\", "\\");
		directory = directory.replace("\\x64\\", "\\");
		hui_win_instance = (HINSTANCE)GetModuleHandle(nullptr);
		directory_static = directory + "static\\";
	#endif

	if (!msg_inited) {
		dir_create(directory);
		msg_init(directory + "message.txt", !(flags & FLAG_SILENT));
	}

	//msg_write("HuiAppDirectory " + HuiAppDirectory);


	_InitInput_();

	ComboBoxSeparator = "\\";
	_using_language_ = false;
	SetDefaultErrorHandler(nullptr);
	//msg_write("");

	Config.filename = directory + "config.txt";


	//msg_write("HuiAppDirectory " + HuiAppDirectory);
	//msg_write("HuiInitialWorkingDirectory " + HuiInitialWorkingDirectory);

	if (flags & FLAG_LOAD_RESOURCE)
		LoadResource(directory_static + "hui_resources.txt");

	if (def_lang.num > 0)
		SetLanguage(Config.get_str("Language", def_lang));

	// at this point:
	//   HuiAppDirectory -> dir to run binary in (binary dir or ~/.my_app/)
	//   HuiAppFilename -> binary file (no dir)
	//   HuiInitialWorkingDirectory -> working dir before running this program
	//   working dir -> ?



	if (file_test_existence(directory_static + "icon.svg"))
		set_property("logo", directory_static + "icon.svg");
	else if (file_test_existence(directory_static + "icon.png"))
		set_property("logo", directory_static + "icon.png");
	else if (file_test_existence(directory_static + "icon.ico"))
		set_property("logo", directory_static + "icon.ico");
}

Application::~Application()
{
	if (Config.changed)
		Config.save();
	if ((msg_inited) /*&& (HuiMainLevel == 0)*/)
		msg_end();
}


// deprecated...
void Application::_init(const Array<string> &arg, const string &program, bool load_res, const string &def_lang)
{
}

int Application::run()
{
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
void Application::end()
{
	SetIdleFunction(nullptr);

	on_end();

	hard_end();
}

// ends the system loop of the run() command
void Application::hard_end()
{
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
		Config.save();
	if (msg_inited)
		msg_end();
}
void Application::do_single_main_loop()
{
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
	do{
		g_main_context_iteration(nullptr, false);
		gtk_main_iteration_do(false);
		counter ++;
		if (counter > 5)
			break;
	}while (gtk_events_pending());

	// pop idle function
	//SetIdleFunction(_if_);
#endif
}


void Application::set_property(const string &name, const string &value)
{
	_properties_.set(name, value);
}

string Application::get_property(const string &name)
{
	try{
		return _properties_[name];
	}catch(...){
		return "";
	}
}

void Application::about_box(Window *win)
{
	AboutBox(win);
}

};
