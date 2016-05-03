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
#include "hui_internal.h"
#include "../file/file.h"


string HuiVersion = "0.5.16.0";

#include <stdio.h>
#include <signal.h>
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

#ifdef OS_WINDOWS
	HINSTANCE hui_win_instance;
#endif
#ifdef HUI_API_WIN
	HFONT hui_win_default_font;
	HICON hui_win_main_icon;
#endif
#ifdef OS_LINUX
	void *_hui_x_display_;
#endif


void _so(const char *str)
{
	printf("%s\n",str);
}

void _so(int i)
{
	printf("%d\n",i);
}




HuiCallback HuiIdleFunction;
HuiCallback HuiErrorFunction;
bool HuiHaveToExit;
bool HuiRunning = false;
bool HuiEndKeepMsgAlive = false;
int HuiMainLevel = -1;
Array<bool> HuiMainLevelRunning;

Array<HuiWindow*> HuiWindows;
Array<HuiClosedPanel> HuiClosedPanels;


bool _HuiScreenOpened_ = false;

// HUI configuration
string HuiComboBoxSeparator;

string HuiAppFilename;
string HuiAppDirectory;			// dir of changeable files (ie. ~/.app/)
string HuiAppDirectoryStatic;	// dir of static files (ie. /usr/shar/app)
string HuiInitialWorkingDirectory;
Array<string> HuiArgument;



#ifdef HUI_API_GTK
	void *invisible_cursor = NULL;
#endif

Array<string> HuiMakeArgs(int num_args, char *args[])
{
	Array<string> a;
	for (int i=0;i<num_args;i++)
		a.add(args[i]);
	HuiArgument = a;
	return a;
}

Array<sHuiImage> HuiImage;





//----------------------------------------------------------------------------------
// system independence of main() function

extern Array<string> HuiMakeArgs(int num_args, char *args[]);


int hui_main(const Array<string> &);

// for a system independent usage of this library

#ifdef OS_WINDOWS

#ifdef _CONSOLE

int _tmain(int NumArgs, _TCHAR *Args[])
{
	return hui_main(HuiMakeArgs(NumArgs, Args));
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
		if ((s[0] == '\"') && (s.back() == '\"'))
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
	return hui_main(HuiMakeArgs(NumArgs, Args));
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




#ifdef HUI_API_GTK
	int idle_id = -1;
	gboolean GtkIdleFunction(void*)
	{
		if (HuiIdleFunction.is_set())
			HuiIdleFunction.call();
		else
			HuiSleep(0.010f);
		return TRUE;
	}

	gboolean GtkRunLaterFunction(gpointer data)
	{
		HuiCallback *c = (HuiCallback*)data;
		c->call();
		delete(c);
		return FALSE;
	}

	gboolean GtkRunRepeatedFunction(gpointer data)
	{
		HuiCallback *c = (HuiCallback*)data;
		c->call();
		return TRUE;
	}
#endif

void _HuiSetIdleFunction(HuiCallback c)
{
#ifdef HUI_API_GTK
	bool old_idle = HuiIdleFunction.is_set();
	bool new_idle = c.is_set();
	if ((new_idle) && (!old_idle))
		idle_id = g_idle_add_full(300, GtkIdleFunction, NULL, NULL);
	if ((!new_idle) && (old_idle) && (idle_id >= 0)){
		g_source_remove(idle_id);
		idle_id = -1;
	}
#endif
	HuiIdleFunction = c;
}

void HuiSetIdleFunction(hui_callback *idle_function)
{
	_HuiSetIdleFunction(idle_function);
}

void _HuiSetIdleFunctionM(HuiEventHandler *object, void (HuiEventHandler::*function)())
{
	_HuiSetIdleFunction(HuiCallback(object, function));
}

int _HuiRunLater(float time, HuiCallback *c)
{
	#ifdef HUI_API_WIN
		msg_todo("HuiRunLater");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		return g_timeout_add_full(300, max((int)(time * 1000), 0), &GtkRunLaterFunction, (void*)c, NULL);
	#endif
}

int HuiRunLater(float time, hui_callback *function)
{
	return _HuiRunLater(time, new HuiCallback(function));
}

int _HuiRunLaterM(float time, HuiEventHandler *object, void (HuiEventHandler::*function)())
{
	return _HuiRunLater(time, new HuiCallback(object, function));
}

int _HuiRunRepeated(float time, HuiCallback *c)
{
	#ifdef HUI_API_WIN
		msg_todo("HuiRunRepeated");
		return 0;
	#endif
	#ifdef HUI_API_GTK
		return g_timeout_add_full(300, max((int)(time * 1000), 0), &GtkRunRepeatedFunction, (void*)c, NULL);
	#endif
}

int HuiRunRepeated(float time, hui_callback *function)
{
	return _HuiRunRepeated(time, new HuiCallback(function));
}

int _HuiRunRepeatedM(float time, HuiEventHandler *object, void (HuiEventHandler::*function)())
{
	return _HuiRunRepeated(time, new HuiCallback(object, function));
}

void HuiCancelRunner(int id)
{
#ifdef HUI_API_GTK
	g_source_remove(id);
#endif
}

void _HuiMakeUsable_()
{
	if (_HuiScreenOpened_)
		return;
#ifdef HUI_API_WIN

	//InitCommonControls(); comctl32.lib
	CoInitialize(NULL);
	//WinStandartFont=CreateFont(8,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"MS Sans Serif");
	hui_win_default_font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

	hui_win_main_icon=ExtractIcon(hui_win_instance,sys_str(HuiAppFilename),0);

#endif
#ifdef HUI_API_GTK
	gtk_init(NULL, NULL);
	#ifdef OS_LINUX
		_hui_x_display_ = XOpenDisplay(0);
	#endif

#if GTK_CHECK_VERSION(3,16,0)
	invisible_cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_BLANK_CURSOR);
#else
	invisible_cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
#endif

#endif
	_HuiScreenOpened_ = true;
}

void HuiInit(const string &program, bool load_res, const string &def_lang)
{
	HuiInitialWorkingDirectory = get_current_dir();

	#ifdef HUI_API_GTK
		g_set_prgname(program.c_str());
	#endif

	#ifdef OS_LINUX
		HuiAppDirectory = HuiInitialWorkingDirectory;
		HuiAppDirectoryStatic = HuiAppDirectory;
		if (HuiArgument.num > 0){
			HuiAppFilename = HuiArgument[0];
			if ((HuiArgument[0].head(5) == "/usr/") or (HuiArgument[0].find("/") < 0)){
				// installed version?
				HuiAppFilename = HuiArgument[0];
				HuiAppDirectory = format("%s/.%s/", getenv("HOME"), program.c_str());
				HuiAppDirectoryStatic = "/usr/share/" + program + "/";
			}
		}
		dir_create(HuiAppDirectory);
	#endif
	#ifdef OS_WINDOWS
		char *ttt = NULL;
		int r = _get_pgmptr(&ttt);
		HuiAppFilename = ttt;
		HuiAppDirectory = HuiAppFilename.dirname();
		HuiAppDirectory = HuiAppDirectory.replace("\\Release\\", "\\");
		HuiAppDirectory = HuiAppDirectory.replace("\\Debug\\", "\\");
		HuiAppDirectory = HuiAppDirectory.replace("\\Unoptimized\\", "\\");
		hui_win_instance = (HINSTANCE)GetModuleHandle(NULL);
		HuiAppDirectoryStatic = HuiAppDirectory;
	#endif

	if (!msg_inited){
		dir_create(HuiAppDirectory);
		msg_init(HuiAppDirectory + "message.txt", true);
	}
	msg_db_f("Hui",1);
	//msg_db_m(format("[%s]", HuiVersion),1);

	//msg_write("HuiAppDirectory " + HuiAppDirectory);



	HuiInitTimers();

	_HuiInitInput_();

	HuiComboBoxSeparator = "\\";
	HuiLanguaged = false;
	HuiPushMainLevel();
	HuiSetDefaultErrorHandler(NULL);
	//msg_write("");

	HuiConfig.filename = HuiAppDirectory + "Data/config.txt";

	
	//msg_write("HuiAppDirectory " + HuiAppDirectory);
	//msg_write("HuiInitialWorkingDirectory " + HuiInitialWorkingDirectory);

	if (load_res)
		HuiLoadResource(HuiAppDirectoryStatic + "Data/hui_resources.txt");

	if (def_lang.num > 0)
		HuiSetLanguage(HuiConfig.getStr("Language", def_lang));

	// at this point:
	//   HuiAppDirectory -> dir to run binary in (binary dir or ~/.my_app/)
	//   HuiAppFilename -> binary file (no dir)
	//   HuiInitialWorkingDirectory -> working dir before running this program
	//   working dir -> ?

	

	if (file_test_existence(HuiAppDirectoryStatic + "Data/icon.svg"))
		HuiSetProperty("logo", HuiAppDirectoryStatic + "Data/icon.svg");
	else if (file_test_existence(HuiAppDirectoryStatic + "Data/icon.ico"))
		HuiSetProperty("logo", HuiAppDirectoryStatic + "Data/icon.ico");
}




// die System-Schleife ausfuehren, Verwendung:
// int main(...)
// {
//     HuiInit();
//     ...
//     return HuiRun();
// }

int HuiRun()
{
	msg_db_f("HuiRun",1);
	HuiRunning = true;
	HuiMainLevelRunning[HuiMainLevel] = true;
	//HuiPushMainLevel();
#ifdef HUI_API_WIN
	MSG messages;
	messages.message = 0;
	HuiHaveToExit = false;
	bool got_message;
	while ((!HuiHaveToExit)&&(WM_QUIT!=messages.message)){
		bool allow=true;
		if (HuiIdleFunction.is_set())
			got_message=(PeekMessage(&messages,NULL,0U,0U,PM_REMOVE)!=0);
		else
			got_message=(GetMessage(&messages,NULL,0,0)!=0);
		if (got_message){
			allow=false;
			TranslateMessage(&messages);
			DispatchMessage(&messages);
			for (int i=0;i<HuiWindows.num;i++)
				if (HuiWindows[i]->hWnd == messages.hwnd){
					allow=true;
					break;
				}
		}
		if ((HuiIdleFunction.is_set()) && (allow))
			HuiIdleFunction.call();
	}
#endif
#ifdef HUI_API_GTK
	gtk_main();
#endif
	return 0;
}

void HuiDoSingleMainLoop()
{
	msg_db_f("HuiDoSingleMainLoop",1);
#ifdef HUI_API_WIN
	MSG messages;
	messages.message=0;
	HuiHaveToExit=false;
	bool got_message;

	bool allow=true;
	if (HuiIdleFunction)
		got_message=(PeekMessage(&messages,NULL,0U,0U,PM_REMOVE)!=0);
	else
		got_message=(GetMessage(&messages,NULL,0,0)!=0);
	if (got_message){
		allow=false;
		TranslateMessage(&messages);
		DispatchMessage(&messages);
		for (int i=0;i<HuiWindows.num;i++)
			if (HuiWindows[i]->hWnd == messages.hwnd){
				allow = true;
				return;
			}
	}
	/*if ((HuiIdleFunction)&&(allow))
		HuiIdleFunction();*/
#endif
#ifdef HUI_API_GTK

	// push idle function
	HuiCallback _if_ = HuiIdleFunction;

	HuiSetIdleFunction(NULL);
	while(gtk_events_pending())
		gtk_main_iteration();

	// pop idle function
	_HuiSetIdleFunction(_if_);
#endif
}

void HuiPushMainLevel()
{
	msg_db_f("HuiPushMainLevel",2);
	HuiMainLevel ++;
	HuiMainLevelRunning.add(false);
}

void HuiCleanUpMainLevel()
{
	msg_db_f("HuiCleanUpMainLevel",2);
	foreachb(HuiWindow *w, HuiWindows)
		if (w->_get_main_level_() >= HuiMainLevel){
			delete(w);
		}
	HuiSetIdleFunction(NULL);
}

void HuiPopMainLevel()
{
	msg_db_f("HuiPopMainLevel",2);
	HuiCleanUpMainLevel();
	HuiMainLevel --;
	
	if (HuiMainLevel < 0)
		HuiSetErrorFunction(NULL);
	else
		HuiMainLevelRunning.pop();
	HuiDoSingleMainLoop();
}

// ends the system loop of the HuiRun() command
void HuiEnd()
{
	msg_db_r("HuiEnd", 1);

	if (HuiMainLevel > 0)
		HuiCleanUpMainLevel();

	// send "quit" message
#ifdef HUI_API_WIN
	PostQuitMessage(0);
#endif
#ifdef HUI_API_GTK
	if (HuiMainLevelRunning.back())
		gtk_main_quit();
#endif

	// really end hui?
	if (HuiMainLevel == 0){
#ifdef HUI_API_GTK
#ifdef OS_LINUX
		// sometimes freezes...
		//if (_hui_x_display_)
		//	XCloseDisplay(hui_x_display);
#endif

#if GTK_CHECK_VERSION(3,0,0)
		g_object_unref(invisible_cursor);
#endif
#endif
		if (HuiConfig.changed)
			HuiConfig.save();
	}
	msg_db_l(1);
	if ((msg_inited) && (!HuiEndKeepMsgAlive) && (HuiMainLevel == 0))
		msg_end();
}


static int _HuiCurrentImageNo_ = 0;

string HuiSetImage(const Image &image)
{
	sHuiImage img;
	img.type = 1;
	img.image = image;
	img.filename = format("image:%d", _HuiCurrentImageNo_ ++);
	HuiImage.add(img);
	return img.filename;
}

void HuiDeleteImage(const string &name)
{
	for (int i=0;i<HuiImage.num;i++)
		if (HuiImage[i].filename == name)
			HuiImage.erase(i);
}



