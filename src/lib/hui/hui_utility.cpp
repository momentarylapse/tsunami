#include "hui.h"


#ifdef _X_USE_NET_
	#include "../net/net.h"
#endif

#ifdef OS_WINDOWS
	#include <direct.h>
	#include <tchar.h>
#endif
#ifdef OS_LINUX
	#include <sys/time.h>
	#include <unistd.h>
#endif


extern hui_callback *HuiIdleFunction, *HuiErrorFunction;
extern Array<HuiWindow*> HuiWindows;

// apply a function to be executed when a critical error occures
void HuiSetErrorFunction(hui_callback *error_function)
{
	HuiErrorFunction=error_function;
	signal(SIGSEGV,(void(*)(int))HuiErrorFunction);
	/*signal(SIGINT,(void (*)(int))HuiErrorFunction);
	signal(SIGILL,(void (*)(int))HuiErrorFunction);
	signal(SIGTERM,(void (*)(int))HuiErrorFunction);
	signal(SIGABRT,(void (*)(int))HuiErrorFunction);*/
	/*signal(SIGFPE,(void (*)(int))HuiErrorFunction);
	signal(SIGBREAK,(void (*)(int))HuiErrorFunction);
	signal(SIGABRT_COMPAT,(void (*)(int))HuiErrorFunction);*/
}

static string _eh_program_, _eh_version_;
static HuiWindow *ErrorDialog,*ReportDialog;
static hui_callback *_eh_cleanup_function_;


#ifdef _X_USE_NET_

void OnReportDialogOK()
{
	string sender = ReportDialog->GetString("report_sender");
	string comment = ReportDialog->GetString("comment");
	string return_msg;
	if (NetSendBugReport(sender, _eh_program_,_eh_version_, comment, return_msg))
		HuiInfoBox(NULL, "ok", return_msg);
	else
		HuiErrorBox(NULL, "error", return_msg);
	delete(ReportDialog);
}

void OnReportDialogClose()
{
	delete(ReportDialog);
}

void HuiSendBugReport()
{
	// dialog
	ReportDialog=HuiCreateDialog(_("Fehlerbericht"),400,295,ErrorDialog,false);
	ReportDialog->AddText(_("!bold\\Name:"),5,5,360,25,"");
	ReportDialog->AddEdit("",5,35,385,25,"report_sender");
	ReportDialog->AddDefButton(_("OK"),265,255,120,25,"ok");
	ReportDialog->SetImage("ok", "hui:ok");
	ReportDialog->AddButton(_("Abbrechen"),140,255,120,25,"cancel");
	ReportDialog->SetImage("cancel", "hui:cancel");
	ReportDialog->AddText(_("!bold\\Kommentar/Geschehnisse:"),5,65,360,25,"");
	ReportDialog->AddMultilineEdit("",5,95,385,110,"comment");
	ReportDialog->AddText(_("Neben diesen Angaben wird noch der Inhalt der Datei message.txt geschickt"),5,210,390,35,"");

	ReportDialog->SetString("report_sender",_("(anonym)"));
	ReportDialog->SetString("comment",_("Ist halt irgendwie passiert..."));

	ReportDialog->Event("ok", &OnReportDialogOK);
	ReportDialog->Event("cancel", &OnReportDialogClose);

	ReportDialog->Run();
}
#endif

void OnErrorDialogShowLog()
{
	HuiOpenDocument("message.txt");
}

void OnErrorDialogClose()
{
	msg_write("real close");
	exit(0);
}

void hui_default_error_handler()
{
	HuiIdleFunction=NULL;

	msg_reset_shift();
	msg_write("");
	msg_write("==============================================================================================");
	msg_write(_("program has crashed, error handler has been called... maybe SegFault... m(-_-)m"));
	//msg_write("---");
	msg_write("      trace:");
	msg_write(msg_get_trace());

	if (_eh_cleanup_function_){
		msg_write(_("i'm now going to clean up..."));
		_eh_cleanup_function_();
		msg_write(_("...done"));
	}

	foreachb(HuiWindow *w, HuiWindows)
		delete(w);
	msg_write(_("                  Close dialog box to exit program."));

	//HuiMultiline=true;
	HuiComboBoxSeparator="$";

	//HuiErrorBox(NULL,"Fehler","Fehler");

	// dialog
	ErrorDialog=HuiCreateDialog(_("Fehler"),600,500,NULL,false);
	ErrorDialog->AddText(_eh_version_ + _(" ist abgest&urzt!		Die letzten Zeilen der Datei message.txt:"),5,5,590,20,"error_header");
	ErrorDialog->AddListView(_("Nachrichten"),5,30,590,420,"message_list");
	//ErrorDialog->AddEdit("",5,30,590,420,"message_list";
	ErrorDialog->AddButton(_("OK"),5,460,100,25,"ok");
	ErrorDialog->SetImage("ok", "hui:ok");
	ErrorDialog->AddButton(_("message.txt &offnen"),115,460,200,25,"show_log");
#ifdef _X_USE_NET_
	ErrorDialog->AddButton(_("Fehlerbericht an Michi senden"),325,460,265,25,"send_report");
	ErrorDialog->Event("send_report", &HuiSendBugReport);
#endif
	for (int i=1023;i>=0;i--){
		string temp = msg_get_str(i);
		if (temp.num > 0)
			ErrorDialog->AddString("message_list", temp);
	}
	ErrorDialog->Event("show_log", &OnErrorDialogShowLog);
	ErrorDialog->Event("cancel", &OnErrorDialogClose);
	ErrorDialog->Event("hui:win_close", &OnErrorDialogClose);
	ErrorDialog->Event("ok", &OnErrorDialogClose);
	ErrorDialog->Run();

	//HuiEnd();
	exit(0);
}

void HuiSetDefaultErrorHandler(const string &program, const string &version, hui_callback *error_cleanup_function)
{
	_eh_cleanup_function_ = error_cleanup_function;
	_eh_program_ = program;
	_eh_version_ = version;
	HuiSetErrorFunction(&hui_default_error_handler);
}

void HuiRaiseError(const string &message)
{
	msg_error(message + " (HuiRaiseError)");
	/*int *p_i=NULL;
	*p_i=4;*/
	hui_default_error_handler();
}






void HuiSleep(int duration_ms)
{
	if (duration_ms<=0)
		return;
#ifdef OS_WINDOWS
	Sleep(duration_ms);
#endif
#ifdef OS_LINUX
	usleep(duration_ms*1000);
#endif
}

// set the default directory
void HuiSetDirectory(const string &dir)
{
#ifdef OS_WINDOWS
	_chdir(sys_str_f(dir));
#endif
#ifdef OS_LINUX
	int r=chdir(sys_str_f(dir));
#endif
}

int HuiGetCpuCount()
{
#ifdef OS_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#endif
#ifdef OS_LINUX
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}



void HuiCopyToClipBoard(const string &buffer)
{
#ifdef HUI_API_WIN
	if (buffer.num < 1)
		return;
	if (!OpenClipboard(NULL))
		return;

	int nn=0; // Anzahl der Zeilenumbrueche
	for (int i=0;i<buffer.num;i++)
		if (buffer[i]=='\n')
			nn++;

	char *str=new char[buffer.num+nn+1];
	HGLOBAL hglbCopy;
	EmptyClipboard();

	// Pointer vorbereiten
	hglbCopy=GlobalAlloc(GMEM_MOVEABLE,sizeof(WCHAR)*(buffer.num+nn+1));
	if (!hglbCopy){
		CloseClipboard();
		return;
	}
	WCHAR *wstr=(WCHAR*)GlobalLock(hglbCopy);

	// befuellen
	int l=0;
	for (i=0;i<buffer.num;i++){
		if (buffer[i]=='\n'){
			str[l]='\r';
			l++;
		}
		str[l]=buffer[i];
		l++;
	}
	str[l+1]=0;

	MultiByteToWideChar(CP_UTF8,0,(LPCSTR)str,-1,wstr,buffer.num+nn+1);
	delete(str);

	GlobalUnlock(hglbCopy);
	SetClipboardData(CF_UNICODETEXT,wstr);
	CloseClipboard();
#endif
#ifdef HUI_API_GTK
	GtkClipboard *cb=gtk_clipboard_get_for_display(gdk_display_get_default(),GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(cb, (char*)buffer.data, buffer.num);
#endif
}

string HuiPasteFromClipBoard()
{
	string r;
#ifdef HUI_API_WIN

	if (!OpenClipboard(NULL))
		return r;
	int nn=0;
	WCHAR *wstr=(WCHAR*)GetClipboardData(CF_UNICODETEXT);
	//char *str=(char*)GetClipboardData(CF_TEXT);
	CloseClipboard();

	int lll=WideCharToMultiByte(CP_UTF8,0,wstr,-1,NULL,0,NULL,NULL)+4;
		//HuiInfoBox(NULL,i2s(lll),"");
	char *str=new char[lll];
	WideCharToMultiByte(CP_UTF8,0,wstr,-1,(LPSTR)str,lll,NULL,NULL);
	delete[](wstr);

	r = str;

	// doppelte Zeilenumbrueche finden
	r.replace("\r", "");
#endif
#ifdef HUI_API_GTK
	//msg_write("--------a");
	GtkClipboard *cb = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
	//msg_write("--------b");
	char *buffer = gtk_clipboard_wait_for_text(cb);
	//msg_write(*buffer);
	if (buffer){
		r = buffer;
		g_free(buffer);
	}
	//msg_write(length);
#endif
	return r;
}

void HuiOpenDocument(const string &filename)
{
#ifdef OS_WINDOWS
	ShellExecute(NULL,_T(""),hui_tchar_str(filename),_T(""),_T(""),SW_SHOW);
#endif
#ifdef OS_LINUX
	int r=system(format("gnome-open '%s'", filename.c_str()).c_str());
#endif
}




//------------------------------------------------------------------------------
// timers


#ifdef OS_WINDOWS
	extern LONGLONG perf_cnt;
	extern bool perf_flag;
	extern float time_scale;
#endif
struct sHuiTimer
{
#ifdef OS_WINDOWS
	LONGLONG CurTime;
	LONGLONG LastTime;
	float time_scale;
#endif
#ifdef OS_LINUX
	struct timeval CurTime, LastTime;
#endif
};
Array<sHuiTimer> HuiTimer;


int HuiCreateTimer()
{
	sHuiTimer t;
	HuiTimer.add(t);
	HuiGetTime(HuiTimer.num - 1); // reset...
	return HuiTimer.num - 1;
}

float HuiGetTime(int index)
{
	/*if (index<0)
		return 0;*/
	float elapsed = 1;
	sHuiTimer *t = &HuiTimer[index];
	#ifdef OS_WINDOWS
		if (perf_flag)
			QueryPerformanceCounter((LARGE_INTEGER *)&t->CurTime);
		else
			t->CurTime = timeGetTime();
		elapsed = (t->CurTime - t->LastTime) * time_scale;
		t->LastTime = t->CurTime;
	#endif
	#ifdef OS_LINUX
		gettimeofday(&t->CurTime,NULL);
		elapsed = float(t->CurTime.tv_sec - t->LastTime.tv_sec) + float(t->CurTime.tv_usec - t->LastTime.tv_usec) * 0.000001f;
		t->LastTime = t->CurTime;
	#endif
	return elapsed;
}

