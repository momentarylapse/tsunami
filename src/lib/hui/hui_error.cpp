/*
 * hui_error.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "hui.h"



#ifdef _X_USE_NET_
	#include "../net/net.h"
#endif

#include <signal.h>

extern HuiCallback HuiIdleFunction, HuiErrorFunction;
extern Array<HuiWindow*> HuiWindows;

void _HuiSignalHandler(int)
{
	HuiErrorFunction.call();
}

// apply a function to be executed when a critical error occures
void HuiSetErrorFunction(hui_callback *error_function)
{
	HuiErrorFunction = error_function;
	if (error_function){
		signal(SIGSEGV, &_HuiSignalHandler);
		/*signal(SIGINT, &_HuiSignalHandler);
		signal(SIGILL, &_HuiSignalHandler);
		signal(SIGTERM, &_HuiSignalHandler);
		signal(SIGABRT, &_HuiSignalHandler);*/
		/*signal(SIGFPE, &_HuiSignalHandler);
		signal(SIGBREAK, &_HuiSignalHandler);
		signal(SIGABRT_COMPAT, &_HuiSignalHandler);*/
	}
}

static HuiWindow *ErrorDialog,*ReportDialog;
static hui_callback *_eh_cleanup_function_;


#ifdef _X_USE_NET_

void OnReportDialogOK()
{
	string sender = ReportDialog->GetString("report_sender");
	string comment = ReportDialog->GetString("comment");
	string return_msg;
	if (NetSendBugReport(sender, HuiGetProperty("name"), HuiGetProperty("version"), comment, return_msg))
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
	ReportDialog = new HuiFixedDialog(_("Fehlerbericht"),400,295,ErrorDialog,false);
	ReportDialog->AddText(_("!bold$Name:"),5,5,360,25,"brd_t_name");
	ReportDialog->AddEdit("",5,35,385,25,"report_sender");
	ReportDialog->AddDefButton(_("OK"),265,255,120,25,"ok");
	ReportDialog->SetImage("ok", "hui:ok");
	ReportDialog->AddButton(_("Abbrechen"),140,255,120,25,"cancel");
	ReportDialog->SetImage("cancel", "hui:cancel");
	ReportDialog->AddText(_("!bold$Kommentar/Geschehnisse:"),5,65,360,25,"brd_t_comment");
	ReportDialog->AddMultilineEdit("",5,95,385,110,"comment");
	ReportDialog->AddText(_("!wrap$Neben diesen Angaben wird noch der Inhalt der Datei message.txt geschickt"),5,210,390,35,"brd_t_explanation");

	ReportDialog->SetString("report_sender",_("(anonym)"));
	ReportDialog->SetString("comment",_("Ist halt irgendwie passiert..."));

	ReportDialog->Event("ok", &OnReportDialogOK);
	ReportDialog->Event("cancel", &OnReportDialogClose);
	ReportDialog->Event("hui:close", &OnReportDialogClose);

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
	msg_write("================================================================================");
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
	HuiComboBoxSeparator = "$";

	//HuiErrorBox(NULL,"Fehler","Fehler");

	// dialog
	ErrorDialog = new HuiFixedDialog(_("Fehler"),600,500,NULL,false);
	ErrorDialog->AddText(HuiGetProperty("name") + " " + HuiGetProperty("version") + _(" ist abgest&urzt!		Die letzten Zeilen der Datei message.txt:"),5,5,590,20,"error_header");
	ErrorDialog->AddListView(_("Nachrichten"),5,30,590,420,"message_list");
	//ErrorDialog->AddEdit("",5,30,590,420,"message_list";
	ErrorDialog->AddButton(_("OK"),5,460,100,25,"ok");
	ErrorDialog->SetImage("ok", "hui:ok");
	ErrorDialog->AddButton(_("message.txt &offnen"),115,460,200,25,"show_log");
	ErrorDialog->AddButton(_("Fehlerbericht an Michi senden"),325,460,265,25,"send_report");
#ifdef _X_USE_NET_
	ErrorDialog->Event("send_report", &HuiSendBugReport);
#else
	ErrorDialog->Enable("send_report", false);
	ErrorDialog->SetTooltip("send_report", _("Anwendung ohne Netzwerk-Unterst&utzung compiliert..."));
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

void HuiSetDefaultErrorHandler(hui_callback *error_cleanup_function)
{
	_eh_cleanup_function_ = error_cleanup_function;
	HuiSetErrorFunction(&hui_default_error_handler);
}

void HuiRaiseError(const string &message)
{
	msg_error(message + " (HuiRaiseError)");
	/*int *p_i=NULL;
	*p_i=4;*/
	hui_default_error_handler();
}




