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


string HuiVersion = "0.4.92.1";


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
		#pragma comment(lib,"gtk-win32-2.0.lib")
		#pragma comment(lib,"glib-2.0.lib")
		#pragma comment(lib,"pango-1.0.lib")
		#pragma comment(lib,"pangowin32-1.0.lib")
		#pragma comment(lib,"cairo.lib")
		#pragma comment(lib,"pangocairo-1.0.lib")
		#pragma comment(lib,"gdk-win32-2.0.lib")
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





hui_callback *HuiIdleFunction = NULL, *HuiErrorFunction = NULL;
HuiEventHandler *hui_idle_object = NULL;
void (HuiEventHandler::*hui_idle_member_function)() = NULL;
bool HuiHaveToExit;
bool HuiRunning;
bool HuiEndKeepMsgAlive = false;
int HuiMainLevel = -1;
Array<bool> HuiMainLevelRunning;

Array<HuiWindow*> HuiWindows;
Array<HuiClosedWindow> _HuiClosedWindow_;


extern bool HuiConfigChanged;
bool _HuiScreenOpened_ = false;

// HUI configuration
string HuiComboBoxSeparator;
bool HuiCreateHiddenWindows;

string HuiAppFilename;
string HuiAppDirectory;			// dir of changeable files (ie. ~/app/)
string HuiAppDirectoryStatic;	// dir of static files (ie. /usr/shar/app)
string HuiInitialWorkingDirectory;
Array<string> HuiArgument;



#ifdef OS_WINDOWS
	LONGLONG perf_cnt;
	bool perf_flag=false;
	float time_scale;
#endif
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


int hui_main(Array<string>);

// for a system independent usage of this library

#ifdef OS_WINDOWS

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	Array<string> a;
	return hui_main(a);
}

#endif
#ifdef OS_LINUX

int main(int NumArgs, char *Args[])
{
	return hui_main(HuiMakeArgs(NumArgs, Args));
}

#endif

// usage:
//
// int hui_main(Array<string> arg)
// {
//     HuiInit();
//     ....
//     return HuiRun();
// }




#ifdef HUI_API_GTK
	int idle_id = -1;
	gboolean GtkIdleFunction(void*)
	{
		if (HuiIdleFunction)
			HuiIdleFunction();
		else if ((hui_idle_object) && (hui_idle_member_function))
			(hui_idle_object->*hui_idle_member_function)();
		else
			HuiSleep(10);
		return TRUE;
	}

	struct HuiRunLaterItem
	{
		hui_callback *function;
		HuiEventHandler *member_object;
		void (HuiEventHandler::*member_function)();

		HuiRunLaterItem()
		{
			function = NULL;
			member_object = NULL;
			member_function = NULL;
		}
	};

	gboolean GtkRunLaterFunction(gpointer data)
	{
		if (data){
			HuiRunLaterItem *i = (HuiRunLaterItem*)data;
			if (i->function)
				i->function();
			else if ((i->member_object) && (i->member_function))
				(i->member_object->*i->member_function)();
			delete(i);
		}
		return false;
	}
#endif

void HuiSetIdleFunction(hui_callback *idle_function)
{
#ifdef HUI_API_GTK
	bool old_idle = (HuiIdleFunction) || ((hui_idle_object) && (hui_idle_member_function));
	bool new_idle = idle_function;
	if ((new_idle) && (!old_idle))
		idle_id = g_idle_add_full(300, GtkIdleFunction, NULL, NULL);
	if ((!new_idle) && (old_idle) && (idle_id >= 0)){
		g_source_remove(idle_id);
		idle_id = -1;
	}
#endif
	hui_idle_object = NULL;
	hui_idle_member_function = NULL;
	HuiIdleFunction = idle_function;
}

void _HuiSetIdleFunctionM(HuiEventHandler *object, void (HuiEventHandler::*function)())
{
#ifdef HUI_API_GTK
	bool old_idle = (HuiIdleFunction) || ((hui_idle_object) && (hui_idle_member_function));
	bool new_idle = ((object) && (function));
	if ((new_idle) && (!old_idle))
		idle_id = g_idle_add_full(300, GtkIdleFunction, NULL, NULL);
	if ((!new_idle) && (old_idle) && (idle_id >= 0)){
		g_source_remove(idle_id);
		idle_id = -1;
	}
#endif
	hui_idle_object = object;
	hui_idle_member_function = function;
	HuiIdleFunction = NULL;
}

void HuiRunLater(float time, hui_callback *function)
{
	#ifdef HUI_API_WIN
		msg_todo("HuiRunLater");
	#endif
	#ifdef HUI_API_GTK
		HuiRunLaterItem *i = new HuiRunLaterItem;
		i->function = function;
		g_timeout_add_full(300, time * 1000, &GtkRunLaterFunction, (void*)i, NULL);
	#endif
}

void _HuiRunLaterM(float time, HuiEventHandler *object, void (HuiEventHandler::*function)())
{
	#ifdef HUI_API_WIN
		msg_todo("HuiRunLater");
	#endif
	#ifdef HUI_API_GTK
		HuiRunLaterItem *i = new HuiRunLaterItem;
		i->member_function = function;
		i->member_object = object;
		g_timeout_add_full(300, time * 1000, &GtkRunLaterFunction, (void*)i, NULL);
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

	hui_win_main_icon=ExtractIcon(hui_win_instance,sys_str(_pgmptr),0);

#endif
#ifdef HUI_API_GTK
	gtk_init(NULL, NULL);
	#ifdef OS_LINUX
		_hui_x_display_ = XOpenDisplay(0);
	#endif

	invisible_cursor = gdk_cursor_new(GDK_BLANK_CURSOR);

#endif
	_HuiScreenOpened_ = true;
}

void HuiInitBase()
{
	#ifdef OS_WINDOWS
		HuiAppFilename = _pgmptr;
		HuiAppDirectory = HuiAppFilename.dirname();
		hui_win_instance = (HINSTANCE)GetModuleHandle(NULL);
	#endif
	#ifdef OS_LINUX
		if (HuiArgument.num > 0){
			if (HuiArgument[0][0] == '.'){
				HuiAppFilename = HuiArgument[0].substr(2, -1);
				HuiAppDirectory = get_current_dir();
			}else{
				HuiAppFilename = HuiArgument[0];
				HuiAppDirectory = HuiAppFilename.dirname();
			}
		}
	#endif

	HuiAppDirectoryStatic = HuiAppDirectory;

	if (!msg_inited){
		dir_create(HuiAppDirectory);
		msg_init(HuiAppDirectory + "message.txt", true);
	}
	HuiRunning = false;

	msg_db_r("Hui",1);
	//msg_db_m(format("[%s]", HuiVersion),1);


	#ifdef OS_WINDOWS
		// timers
		if (QueryPerformanceFrequency((LARGE_INTEGER *) &perf_cnt)){
			perf_flag=true;
			time_scale=1.0f/perf_cnt;
		}else 
			time_scale=0.001f;

	#endif

	_HuiInitInput_();

	HuiComboBoxSeparator="\\";
	HuiIdleFunction=NULL;
	HuiErrorFunction=NULL;
	HuiLanguaged=false;
	HuiCreateHiddenWindows=false;

	HuiPushMainLevel();

	msg_db_l(1);
}

void HuiInit(const string &program, bool load_res, const string &def_lang)
{
	HuiInitialWorkingDirectory = get_current_dir();
	string s1, s2;

	#ifdef HUI_API_GTK
		g_set_prgname(program.c_str());
	#endif

	#ifdef OS_LINUX
		if (HuiArgument.num > 0){
			if (HuiArgument[0][0] == '/'){
				if (HuiArgument[0][1] == 'u'){ // /usr/...
					HuiAppFilename = HuiArgument[0];
					HuiAppDirectory = format("%s/.%s/", getenv("HOME"), program.c_str());
					HuiAppDirectoryStatic = "/usr/share/" + program + "/";
				}else{
					HuiAppFilename = HuiArgument[0];
					HuiAppDirectory = HuiAppFilename.dirname();
					HuiAppDirectoryStatic = HuiAppDirectory;
				}
			}else{
				HuiAppDirectory.dir_ensure_ending();
				if (HuiArgument[0][0] == '.'){
					HuiAppFilename = HuiArgument[0].substr(2, -1);
					HuiAppDirectory = HuiInitialWorkingDirectory;
					HuiAppDirectoryStatic = HuiAppDirectory;
				}else{
					HuiAppFilename = HuiArgument[0];
					HuiAppDirectory = format("%s/.%s/", getenv("HOME"), program.c_str());
					HuiAppDirectoryStatic = "/usr/share/" + program + "/";
				}
			}
		}
		s1 = HuiAppDirectory;
		s2 = HuiAppDirectoryStatic;
	#else
		HuiAppDirectory = HuiInitialWorkingDirectory;
		HuiAppDirectoryStatic = HuiAppDirectory;
	#endif

	if (!msg_inited){
		dir_create(HuiAppDirectory);
		msg_init(HuiAppDirectory + "message.txt", true);
	}

	//msg_write("HuiAppDirectory " + HuiAppDirectory);
		

	HuiInitBase();
#ifdef OS_LINUX
	HuiAppDirectory = s1;
	HuiAppDirectoryStatic = s2;
	dir_create(HuiAppDirectory);
#endif
	HuiSetDefaultErrorHandler(NULL);
	//msg_write("");

	
	//msg_write("HuiAppDirectory " + HuiAppDirectory);
	//msg_write("HuiInitialWorkingDirectory " + HuiInitialWorkingDirectory);

	if (load_res)
		HuiLoadResource(HuiAppDirectoryStatic + "Data/hui_resources.txt");

	if (def_lang.num > 0)
		HuiSetLanguage(HuiConfigReadStr("Language", def_lang));

	// at this point:
	//   HuiAppDirectory -> dir to run binary in (binary dir or ~/.my_app/)
	//   HuiAppFilename -> binary file (no dir)
	//   HuiInitialWorkingDirectory -> working dir before running this program
	//   working dir -> ?

	

	if (file_test_existence(HuiAppDirectoryStatic + "Data/icon.svg"))
		HuiSetProperty("logo", HuiAppDirectoryStatic + "Data/icon.svg");
	else if (file_test_existence(HuiAppDirectoryStatic + "Data/icon.ico"))
		HuiSetProperty("logo", HuiAppDirectoryStatic + "Data/icon.ico");
	if (file_test_existence(HuiAppDirectoryStatic + "Data/license_small.txt"))
		HuiSetProperty("license", FileRead(HuiAppDirectoryStatic + "Data/license_small.txt"));
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
	msg_db_r("HuiRun",1);
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
					allow=true;
					break;
				}
		}
		if ((HuiIdleFunction)&&(allow))
			HuiIdleFunction();
	}
#endif
#ifdef HUI_API_GTK
	gtk_main();
#endif
	msg_db_l(1);
	return 0;
}

void HuiDoSingleMainLoop()
{
	msg_db_r("HuiDoSingleMainLoop",1);
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
	hui_callback *_if_ = HuiIdleFunction;
	HuiEventHandler *_io_ = hui_idle_object;
	void (HuiEventHandler::*_imf_)() = hui_idle_member_function;

	HuiSetIdleFunction(NULL);
	while(gtk_events_pending())
		gtk_main_iteration();

	// pop idle function
	if (_if_)
		HuiSetIdleFunction(_if_);
	else if ((_io_) && (_imf_))
		HuiSetIdleFunctionM(_io_, _imf_);
#endif
	msg_db_l(1);
}

void HuiPushMainLevel()
{
	msg_db_r("HuiPushMainLevel",2);
	HuiMainLevel ++;
	HuiMainLevelRunning.add(false);
	msg_db_l(2);
}

void HuiCleanUpMainLevel()
{
	msg_db_r("HuiCleanUpMainLevel",2);
	foreachb(HuiWindow *w, HuiWindows)
		if (w->_GetMainLevel_() >= HuiMainLevel)
			delete(w);
	HuiSetIdleFunction(NULL);
	msg_db_l(2);
}

void HuiPopMainLevel()
{
	msg_db_r("HuiPopMainLevel",2);
	HuiCleanUpMainLevel();
	HuiMainLevel --;
	
	if (HuiMainLevel < 0)
		HuiSetErrorFunction(NULL);
	else
		HuiMainLevelRunning.pop();
	HuiDoSingleMainLoop();
	msg_db_l(2);
}

// ends the system loop of the HuiRun() command
void HuiEnd()
{
	msg_db_r("HuiEnd",1);

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

#if GTK_MAJOR_VERSION == 3
		g_object_unref(invisible_cursor);
#endif
#endif
		if (HuiConfigChanged)
			HuiSaveConfigFile();
	}
	msg_db_l(1);
	if ((msg_inited) && (!HuiEndKeepMsgAlive) && (HuiMainLevel == 0))
		msg_end();
}


string HuiSetImage(const Image &image)
{
	sHuiImage img;
	img.type = 1;
	img.image = image;
	img.filename = format("image:%d", HuiImage.num);
	HuiImage.add(img);
	return img.filename;
}



