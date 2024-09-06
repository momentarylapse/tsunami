/*
 * hui_error.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "hui.h"
#include "internal.h"
#include "../os/msg.h"

#ifdef _X_USE_NET_
	#include "../net/net.h"
#endif

#include <signal.h>

namespace hui
{

extern Callback _idle_function_, _error_function_;
extern bool _screen_opened_;

static void _hui_signal_handler(int s) {
	_error_function_();
}


// apply a function to be executed when a critical error occures
void SetErrorFunction(const Callback &function) {
	_error_function_ = function;
	if (function) {
		signal(SIGSEGV, &_hui_signal_handler);
		//signal(SIGINT, &_hui_signal_handler);
		signal(SIGILL, &_hui_signal_handler);
		/*signal(SIGTERM, &_hui_signal_handler);
		signal(SIGABRT, &_hui_signal_handler);
		signal(SIGFPE, &_hui_signal_handler);
		signal(SIGBREAK, &_hui_signal_handler);
		signal(SIGABRT_COMPAT, &_hui_signal_handler);*/
	}
}

static Callback _eh_cleanup_function_;


#ifdef _X_USE_NET_

class ReportDialog : public Dialog {
public:
	ReportDialog(Window *parent) :
		Dialog(_("Bug report"), 600, 500, parent, false)
	{
		set_options("", "headerbar,resizable,closebutton=no");
		set_title(_("Bug report"));
		set_info_text("this does not work anymore... I've changed my server", {"warning"});

		add_grid("", 0, 0, "root");
		set_target("root");

		add_group(_("Name:"), 0, 0, "grp-name");
		add_group(_("Comment / what happened:"), 0, 1, "grp-comment");
		add_label("!wrap//" + _("Your comments and the contents of the file message.txt will be sent."), 0, 2, "t-explanation");
		add_grid("!buttonbar", 0, 3, "buttonbar");

		set_target("grp-name");
		add_edit(_("(anonymous)"), 0, 0, "sender");

		set_target("grp-comment");
		add_multiline_edit("!expandy,expandx//", 0, 0, "comment");
		set_string("comment", _("Just happened somehow..."));

		set_target("buttonbar");
		add_button(_("Cancel"),0, 0,"cancel");
		set_image("cancel", "hui:cancel");
		add_button("!default,\\" + _("Ok"), 0, 1 ,"ok");
		set_image("ok", "hui:ok");

		event("ok", [this]{ on_ok(); });
		event("cancel", [this]{ request_destroy(); });
		event("hui:close", [this]{ request_destroy(); });
	}

	void on_ok() {
		string sender = get_string("sender");
		string comment = get_string("comment");
		string return_msg;
		try {
			NetSendBugReport(sender, Application::get_property("name"), Application::get_property("version"), comment);
			info_box(nullptr, "ok", return_msg);
		} catch (Exception &e) {
			error_box(nullptr, "error", e.message());
		}
		request_destroy();
	}
};

void SendBugReport(Window *parent) {
	hui::fly(new ReportDialog(parent));
}

#endif

class ErrorDialog : public Dialog {
public:
	ErrorDialog() :
		Dialog(_("Error"), 800, 600, nullptr, false)
	{
		set_options("", "headerbar,resizable,closebutton=no");
		set_title(_("Error"));
		add_grid("", 0, 0, "root");
		set_target("root");
		add_label(Application::get_property("name") + " " + Application::get_property("version") + _(" has crashed.		The last lines of the file message.txt:"), 0, 0, "error_header");
		add_list_view("!nobar", 0, 1, "message-list");
		add_grid("!buttonbar", 0, 2, "buttonbar");
		set_target("buttonbar");
		if (false) {
			add_button(_("Open log"), 0, 0, "show-log");
			set_tooltip("show-log", _("open the 'message.txt' file in a text editor"));
		}
		add_button(_("Bug report"), 1, 0, "send-report");
		set_tooltip("send-report", _("send a bug report to Michi"));
		add_button(_("Ok"), 0, 0, "ok");
		set_image("ok", "hui:ok");
		set_options("ok", "danger,default");
		set_tooltip("ok", _("will close the program"));

	#ifdef _X_USE_NET_
		event("send-report", [this] {
			SendBugReport(this);
		});
	#else
		enable("send-report", false);
		set_tooltip("send-report", _("Program was compiled without network support..."));
	#endif

		int n = 0;
		for (int i=1023;i>=0;i--){
			string temp = msg_get_str(i);
			if (temp.num > 0) {
				add_string("message-list", temp);
				n ++;
			}
		}
		set_int("message-list", n-1);

		event("show-log", [] {
			open_document("message.txt");
		});
		event("hui:win_close", [] {
			exit(1);
		});
		event("ok", [] {
			exit(1);
		});
	}
};

void show_crash_window() {
	hui::fly_and_wait(new ErrorDialog);
}

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

	foreachb(Window *w, _all_windows_) {
		w->hide();
	}
	//	delete w;

	msg_write(_("                  Close dialog box to exit program."));

	//HuiMultiline=true;
	//ComboBoxSeparator = "$";

	//HuiErrorBox(NULL,"Fehler","Fehler");

	// dialog
	if (_screen_opened_)
		show_crash_window();

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

