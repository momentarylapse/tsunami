/*----------------------------------------------------------------------------*\
| CHui                                                                         |
| -> Heroic User Interface                                                     |
| -> abstraction layer for GUI                                                 |
|   -> Windows (default api) or Linux (Gtk+)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.06.21 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/


#include "hui.h"
#include "../file/file.h"


#include <stdio.h>
#include <signal.h>

#include "internal.h"
#ifdef OS_WINDOWS
	#include <shlobj.h>
	#include <winuser.h>
	#include <direct.h>
	#include <commctrl.h>
	#include <tchar.h>
	#pragma comment(lib,"winmm.lib")
	#ifdef HUI_API_GTK
		#pragma comment(lib,"gtk-win32-3.0.lib")
		#pragma comment(lib,"glib-2.0.lib")
		#pragma comment(lib,"pango-1.0.lib")
		#pragma comment(lib,"pangowin32-1.0.lib")
		#pragma comment(lib,"cairo.lib")
		#pragma comment(lib,"pangocairo-1.0.lib")
		#pragma comment(lib,"gdk-win32-3.0.lib")
		#pragma comment(lib,"gdk_pixbuf-2.0.lib")
		#pragma comment(lib,"gobject-2.0.lib")
	#else
		#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#endif
	#pragma warning(disable : 4995)
#endif
#ifdef OS_LINUX
	#include <gdk/gdkx.h>
#endif

namespace hui
{


string Version = "0.6.0.0";


#ifdef OS_WINDOWS
	HINSTANCE hui_win_instance;
#endif
#ifdef HUI_API_WIN
	HFONT hui_win_default_font;
	HICON hui_win_main_icon;
#endif
#ifdef OS_LINUX
	Display* x_display;
#endif




Callback _idle_function_;
Callback _error_function_;
bool Running = false;
bool EndKeepMsgAlive = false;
int _main_level_ = -1;
Array<bool> _main_level_running_;

Array<Window*> _hui_windows_;


bool _screen_opened_ = false;

// HUI configuration
string ComboBoxSeparator;

string AppFilename;
string AppDirectory;			// dir of changeable files (ie. ~/.app/)
string AppDirectoryStatic;	// dir of static files (ie. /usr/shar/app)
string InitialWorkingDirectory;
Array<string> Argument;



#ifdef HUI_API_GTK
	void *invisible_cursor = NULL;
#endif

Array<string> MakeArgs(int num_args, char *args[])
{
	Array<string> a;
	for (int i=0;i<num_args;i++)
		a.add(args[i]);
	Argument = a;
	return a;
}

Array<HuiImage> _hui_images_;





//----------------------------------------------------------------------------------
// system independence of main() function

extern Array<string> MakeArgs(int num_args, char *args[]);


}

int hui_main(const Array<string> &);

// for a system independent usage of this library

#ifdef OS_WINDOWS

#ifdef _CONSOLE

int _tmain(int NumArgs, _TCHAR *Args[])
{
	return hui_main(MakeArgs(NumArgs, Args));
}

#else

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	Array<string> a;
	a.add("-dummy-");
	string s = lpCmdLine;
	if (s.num > 0){
		if ((s[0] == '\"') and (s.back() == '\"'))
			s = s.substr(1, -2);
		a.add(s);
	}
	return hui_main(a);
}

#endif

#endif
#ifdef OS_LINUX

int main(int NumArgs, char *Args[])
{
	return hui_main(hui::MakeArgs(NumArgs, Args));
}

#endif

// usage:
//
// int hui_main(const Array<string> &arg)
// {
//     HuiInit();
//     ....
//     return HuiRun();
// }


namespace hui
{


#ifdef HUI_API_GTK
	int idle_id = -1;

	class HuiGtkRunner
	{
	public:
		HuiGtkRunner(const Callback &_func)
		{
			func = _func;
			id = -1;
		}
		Callback func;
		int id;
	};
	Array<HuiGtkRunner*> _hui_runners_;
	void _hui_runner_delete_(int id)
	{
		for (int i=_hui_runners_.num-1; i>=0; i--)
			if (_hui_runners_[i]->id == id){
				delete _hui_runners_[i];
				_hui_runners_.erase(i);
			}
	}

	gboolean GtkIdleFunction(void*)
	{
		if (_idle_function_)
			_idle_function_();
		else
			Sleep(0.010f);
		return TRUE;
	}

	gboolean GtkRunLaterFunction(gpointer data)
	{
		HuiGtkRunner *c = reinterpret_cast<HuiGtkRunner*>(data);
		if (c->func)
			c->func();
		_hui_runner_delete_(c->id);
		return FALSE;
	}

	gboolean GtkRunRepeatedFunction(gpointer data)
	{
		HuiGtkRunner *c = reinterpret_cast<HuiGtkRunner*>(data);
		if (c->func)
			c->func();
		return TRUE;
	}
#endif

void SetIdleFunction(const Callback &c)
{
#ifdef HUI_API_GTK
	bool old_idle = (bool)_idle_function_;
	bool new_idle = (bool)c;
	if (new_idle and !old_idle)
		idle_id = g_idle_add_full(300, GtkIdleFunction, NULL, NULL);
	if (!new_idle and old_idle and (idle_id >= 0)){
		g_source_remove(idle_id);
		idle_id = -1;
	}
#endif
	_idle_function_ = c;
}

/*void HuiSetIdleFunction(hui_callback *idle_function)
{
	_HuiSetIdleFunction(idle_function);
}

void _HuiSetIdleFunctionM(HuiEventHandler *object, void (HuiEventHandler::*function)())
{
	_HuiSetIdleFunction(HuiCallback(object, function));
}*/

int RunLater(float time, const Callback &c)
{
	#ifdef HUI_API_WIN
		msg_todo("HuiRunLater");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		HuiGtkRunner *r = new HuiGtkRunner(c);
		r->id = g_timeout_add_full(300, max((int)(time * 1000), 0), &GtkRunLaterFunction, (void*)r, NULL);
		_hui_runners_.add(r);
		return r->id;
	#endif
}

/*int HuiRunLater(float time, hui_callback *function)
{
	return _HuiRunLater(time, new HuiCallback(function));
}

int _HuiRunLaterM(float time, HuiEventHandler *object, void (HuiEventHandler::*function)())
{
	return _HuiRunLater(time, new HuiCallback(object, function));
}*/

int RunRepeated(float time, const Callback &c)
{
	#ifdef HUI_API_WIN
		msg_todo("HuiRunRepeated");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		HuiGtkRunner *r = new HuiGtkRunner(c);
		r->id = g_timeout_add_full(300, max((int)(time * 1000), 0), &GtkRunRepeatedFunction, (void*)r, NULL);
		_hui_runners_.add(r);
		return r->id;
	#endif
}

/*int HuiRunRepeated(float time, hui_callback *function)
{
	return _HuiRunRepeated(time, new HuiCallback(function));
}

int _HuiRunRepeatedM(float time, HuiEventHandler *object, void (HuiEventHandler::*function)())
{
	return _HuiRunRepeated(time, new HuiCallback(object, function));
}*/

void CancelRunner(int id)
{
#ifdef HUI_API_GTK
	g_source_remove(id);
	_hui_runner_delete_(id);
#endif
}

void _MakeUsable_()
{
	if (_screen_opened_)
		return;
#ifdef HUI_API_WIN

	//InitCommonControls(); comctl32.lib
	CoInitialize(NULL);
	//WinStandartFont=CreateFont(8,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"MS Sans Serif");
	hui_win_default_font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

	hui_win_main_icon=ExtractIcon(hui_win_instance,sys_str(AppFilename),0);

#endif
#ifdef HUI_API_GTK
	gtk_init(NULL, NULL);
	#ifdef OS_LINUX
		x_display = XOpenDisplay(0);
	#endif

#if GTK_CHECK_VERSION(3,16,0)
	invisible_cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_BLANK_CURSOR);
#else
	invisible_cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
#endif

#endif
	_screen_opened_ = true;
}

void Init(const string &program, bool load_res, const string &def_lang)
{
	InitialWorkingDirectory = get_current_dir();

	#ifdef HUI_API_GTK
		g_set_prgname(program.c_str());
	#endif

	#ifdef OS_LINUX
		AppDirectory = InitialWorkingDirectory;
		AppDirectoryStatic = AppDirectory;
		if (Argument.num > 0){
			AppFilename = Argument[0];
			if ((Argument[0].head(5) == "/usr/") or (Argument[0].find("/") < 0)){
				// installed version?
				AppFilename = Argument[0];
				AppDirectory = format("%s/.%s/", getenv("HOME"), program.c_str());
				AppDirectoryStatic = "/usr/share/" + program + "/";
			}
		}
		dir_create(AppDirectory);
	#endif
	#ifdef OS_WINDOWS
		char *ttt = NULL;
		int r = _get_pgmptr(&ttt);
		AppFilename = ttt;
		AppDirectory = AppFilename.dirname();
		AppDirectory = AppDirectory.replace("\\Release\\", "\\");
		AppDirectory = AppDirectory.replace("\\Debug\\", "\\");
		AppDirectory = AppDirectory.replace("\\Unoptimized\\", "\\");
		hui_win_instance = (HINSTANCE)GetModuleHandle(NULL);
		AppDirectoryStatic = AppDirectory;
	#endif

	if (!msg_inited){
		dir_create(AppDirectory);
		msg_init(AppDirectory + "message.txt", true);
	}

	//msg_write("HuiAppDirectory " + HuiAppDirectory);



	InitTimers();

	_InitInput_();

	ComboBoxSeparator = "\\";
	_using_language_ = false;
	PushMainLevel();
	SetDefaultErrorHandler(NULL);
	//msg_write("");

	Config.filename = AppDirectory + "Data/config.txt";

	
	//msg_write("HuiAppDirectory " + HuiAppDirectory);
	//msg_write("HuiInitialWorkingDirectory " + HuiInitialWorkingDirectory);

	if (load_res)
		LoadResource(AppDirectoryStatic + "Data/hui_resources.txt");

	if (def_lang.num > 0)
		SetLanguage(Config.getStr("Language", def_lang));

	// at this point:
	//   HuiAppDirectory -> dir to run binary in (binary dir or ~/.my_app/)
	//   HuiAppFilename -> binary file (no dir)
	//   HuiInitialWorkingDirectory -> working dir before running this program
	//   working dir -> ?

	

	if (file_test_existence(AppDirectoryStatic + "Data/icon.svg"))
		SetProperty("logo", AppDirectoryStatic + "Data/icon.svg");
	else if (file_test_existence(AppDirectoryStatic + "Data/icon.ico"))
		SetProperty("logo", AppDirectoryStatic + "Data/icon.ico");
}




// die System-Schleife ausfuehren, Verwendung:
// int main(...)
// {
//     HuiInit();
//     ...
//     return HuiRun();
// }

int Run()
{
	Running = true;
	_main_level_running_[_main_level_] = true;
	//HuiPushMainLevel();
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

void DoSingleMainLoop()
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
	Callback _if_ = _idle_function_;

	SetIdleFunction(NULL);
	while(gtk_events_pending())
		gtk_main_iteration();

	// pop idle function
	SetIdleFunction(_if_);
#endif
}

void PushMainLevel()
{
	_main_level_ ++;
	_main_level_running_.add(false);
}

void CleanUpMainLevel()
{
	foreachb(Window *w, _hui_windows_)
		if (w->_get_main_level_() >= _main_level_){
			delete(w);
		}
	SetIdleFunction(NULL);
}

void PopMainLevel()
{
	CleanUpMainLevel();
	_main_level_ --;
	
	if (_main_level_ < 0)
		SetErrorFunction(NULL);
	else
		_main_level_running_.pop();
	DoSingleMainLoop();
}

// ends the system loop of the HuiRun() command
void End()
{
	if (_main_level_ > 0)
		CleanUpMainLevel();

	// send "quit" message
#ifdef HUI_API_WIN
	PostQuitMessage(0);
#endif
#ifdef HUI_API_GTK
	if (_main_level_running_.back())
		gtk_main_quit();
#endif

	// really end hui?
	if (_main_level_ == 0){
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
	}
	if ((msg_inited) and (!EndKeepMsgAlive) and (_main_level_ == 0))
		msg_end();
}


static int _current_image_no_ = 0;

string SetImage(const Image &image)
{
	HuiImage img;
	img.type = 1;
	img.image = image;
	img.filename = format("image:%d", _current_image_no_ ++);
	_hui_images_.add(img);
	return img.filename;
}

void DeleteImage(const string &name)
{
	for (int i=0;i<_hui_images_.num;i++)
		if (_hui_images_[i].filename == name)
			_hui_images_.erase(i);
}


};
