/*
 * hui_error.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "hui.h"
#include "internal.h"



#ifdef _X_USE_NET_
	#include "../net/net.h"
#endif

#include <signal.h>

namespace hui
{

extern Callback _idle_function_, _error_function_;
extern bool _screen_opened_;

void _HuiSignalHandler(int) {
	_error_function_();
}

// apply a function to be executed when a critical error occures
void SetErrorFunction(const Callback &function) {
	_error_function_ = function;
	if (function) {
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

static Callback _eh_cleanup_function_;


#ifdef _X_USE_NET_

class ReportDialog : public Dialog {
public:
	ReportDialog(Window *parent) :
		Dialog(_("Bug Report"), 450, 400, parent, false)
	{
		add_grid("", 0, 0, "root");
		set_target("root");

		add_group(_("Name:"), 0, 0, "grp_name");
		add_group(_("Comment/what happened:"), 0, 1, "grp_comment");
		add_label("!wrap//" + _("Your comments and the contents of the file message.txt will be sent."), 0, 2, "t_explanation");
		add_grid("!buttonbar", 0, 3, "buttonbar");

		set_target("grp_name");
		add_edit(_("(anonymous)"), 0, 0, "sender");

		set_target("grp_comment");
		add_multiline_edit("!expandy,expandx//" + _("Just happened somehow..."), 0, 0, "comment");

		set_target("buttonbar");
		add_button(_("Cancel"),0, 0,"cancel");
		set_image("cancel", "hui:cancel");
		add_def_button(_("Ok"), 1, 0 ,"ok");
		set_image("ok", "hui:ok");

		event("ok", [=]{ on_ok(); });
		event("cancel", [=]{ request_destroy(); });
		event("hui:close", [=]{ request_destroy(); });
	}

	void on_ok() {
		string sender = get_string("sender");
		string comment = get_string("comment");
		string return_msg;
		try {
			NetSendBugReport(sender, Application::get_property("name"), Application::get_property("version"), comment);
			InfoBox(nullptr, "ok", return_msg);
		} catch (Exception &e) {
			ErrorBox(nullptr, "error", e.message());
		}
		request_destroy();
	}
};

void SendBugReport(Window *parent) {
	ReportDialog *dlg = new ReportDialog(parent);
	dlg->run();
	delete(dlg);
}

#endif

class ErrorDialog : public Dialog {
public:
	ErrorDialog() :
		Dialog(_("Error"), 600, 500, nullptr, false)
	{
		add_grid("", 0, 0, "root");
		set_target("root");
		add_label(Application::get_property("name") + " " + Application::get_property("version") + _(" has crashed.		The last lines of the file message.txt:"), 0, 0, "error_header");
		add_list_view(_("Messages"), 0, 1, "message_list");
		add_grid("!buttonbar", 0, 2, "buttonbar");
		set_target("buttonbar");
		add_button(_("open message.txt"), 0, 0, "show_log");
		add_button(_("Send bug report to Michi"), 1, 0, "send_report");
		add_button(_("Ok"), 2, 0, "ok");
		set_image("ok", "hui:ok");

	#ifdef _X_USE_NET_
		event("send_report", std::bind(&ErrorDialog::on_send_bug_report, this));
	#else
		enable("send_report", false);
		set_tooltip("send_report", _("Program was compiled without network support..."));
	#endif
		for (int i=1023;i>=0;i--){
			string temp = msg_get_str(i);
			if (temp.num > 0)
				add_string("message_list", temp);
		}
		event("show_log", [=]{ on_show_log(); });
		//event("cancel", std::bind(&ErrorDialog::onClose, this));
		event("hui:win_close", [=]{ on_close(); });
		event("ok", [=]{ on_close(); });
	}

	void on_show_log() {
		OpenDocument("message.txt");
	}

	void on_send_bug_report() {
		SendBugReport(this);
	}

	void on_close() {
		msg_write("real close");
		exit(0);
	}
};

void hui_default_error_handler() {
	_idle_function_ = nullptr;

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

	foreachb(Window *w, _all_windows_)
		delete w;
	msg_write(_("                  Close dialog box to exit program."));

	//HuiMultiline=true;
	//ComboBoxSeparator = "$";

	//HuiErrorBox(NULL,"Fehler","Fehler");

	// dialog
	if (_screen_opened_) {
		ErrorDialog *dlg = new ErrorDialog;
		dlg->run();
	}

	//HuiEnd();
	exit(1);
}

void SetDefaultErrorHandler(const Callback &error_cleanup_function) {
	_eh_cleanup_function_ = error_cleanup_function;
	SetErrorFunction(&hui_default_error_handler);
}

void RaiseError(const string &message) {
	msg_error(message + " (HuiRaiseError)");
	/*int *p_i=NULL;
	*p_i=4;*/
	hui_default_error_handler();
}


};

