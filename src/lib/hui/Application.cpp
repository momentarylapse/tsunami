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



Application::Application(const string &app_name, const string &def_lang, int flags)
{
	initial_working_directory = get_current_dir();
	installed = false;

	#ifdef HUI_API_GTK
		g_set_prgname(app_name.c_str());
	#endif

	#ifdef OS_LINUX
		directory = initial_working_directory;
		directory_static = directory + "static/";
		if (_args.num > 0){
			filename = _args[0];
			if ((_args[0].head(5) == "/usr/") or (_args[0].find("/") < 0)){
				installed = true;
				// installed version?
				filename = _args[0];
				directory = format("%s/.%s/", getenv("HOME"), app_name.c_str());
				directory_static = "/usr/share/" + app_name + "/";
			}
		}
		dir_create(directory);
	#endif
	#ifdef OS_WINDOWS
		char *ttt = NULL;
		int r = _get_pgmptr(&ttt);
		filename = ttt;
		directory = filename.dirname();
		directory = directory.replace("\\Release\\", "\\");
		directory = directory.replace("\\Debug\\", "\\");
		directory = directory.replace("\\Unoptimized\\", "\\");
		hui_win_instance = (HINSTANCE)GetModuleHandle(NULL);
		directory_static = directory;
	#endif

	if (!msg_inited){
		dir_create(directory);
		msg_init(directory + "message.txt", !(flags & FLAG_SILENT));
	}

	//msg_write("HuiAppDirectory " + HuiAppDirectory);



	InitTimers();

	_InitInput_();

	ComboBoxSeparator = "\\";
	_using_language_ = false;
	SetDefaultErrorHandler(NULL);
	//msg_write("");

	Config.filename = directory + "config.txt";


	//msg_write("HuiAppDirectory " + HuiAppDirectory);
	//msg_write("HuiInitialWorkingDirectory " + HuiInitialWorkingDirectory);

	if (flags & FLAG_LOAD_RESOURCE)
		LoadResource(directory_static + "hui_resources.txt");

	if (def_lang.num > 0)
		SetLanguage(Config.getStr("Language", def_lang));

	// at this point:
	//   HuiAppDirectory -> dir to run binary in (binary dir or ~/.my_app/)
	//   HuiAppFilename -> binary file (no dir)
	//   HuiInitialWorkingDirectory -> working dir before running this program
	//   working dir -> ?



	if (file_test_existence(directory_static + "icon.svg"))
		setProperty("logo", directory_static + "icon.svg");
	else if (file_test_existence(directory_static + "icon.ico"))
		setProperty("logo", directory_static + "icon.ico");
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
			got_message=(PeekMessage(&messages,NULL,0U,0U,PM_REMOVE)!=0);
		else
			got_message=(GetMessage(&messages,NULL,0,0)!=0);
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
	SetIdleFunction(NULL);

	onEnd();

	hard_end();
}

// ends the system loop of the run() command
void Application::hard_end()
{
	SetIdleFunction(NULL);

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
void Application::doSingleMainLoop()
{
#ifdef HUI_API_WIN
	MSG messages;
	messages.message=0;
	HuiHaveToExit=false;
	bool got_message;

	bool allow=true;
	if (_idle_function_)
		got_message=(PeekMessage(&messages,NULL,0U,0U,PM_REMOVE)!=0);
	else
		got_message=(GetMessage(&messages,NULL,0,0)!=0);
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
	do{
		gtk_main_iteration();
	}while (gtk_events_pending());

	// pop idle function
	//SetIdleFunction(_if_);
#endif
}


void Application::setProperty(const string &name, const string &value)
{
	_properties_[name] = value;
}

string Application::getProperty(const string &name)
{
	return _properties_[name];
}

void Application::aboutBox(Window *win)
{
	AboutBox(win);
}

};
