/*
 * Progress.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "Progress.h"
#include "../../lib/hui/hui.h"

namespace tsunami {


const float PROGRESS_DT = 0.05f;

Progress::Progress(const string &str, hui::Window *parent) : Progress() {
	if (parent) {
		dlg = hui::create_resource_dialog("progress_dialog", parent);
		dlg->event("hui:close", []{}); // ignore
		set(str, 0);
		dlg->show();
	}
}

Progress::Progress() {
	cancelled = false;
	allow_next = 0;
	timer = new os::Timer;
}


void Progress::__init__(const string &str, hui::Window *parent) {
	new(this) Progress(str, parent);
}

void Progress::__delete__() {
	this->Progress::~Progress();
}


void Progress::set(const string &str, float progress) {
	if (dlg) {
		float t = timer->peek();
		if (t < allow_next)
			return;
		if (str.num > 0)
			dlg->set_title(str);
		if (t > 2) {
			int eta = (int)(t / progress * (1-progress) + 0.7f);
			if (eta >= 60)
				dlg->set_string("progress_bar", format("%.1f%% (%dmin %ds)", progress * 100, eta/60, eta %60));
			else
				dlg->set_string("progress_bar", format("%.1f%% (%ds)", progress * 100, eta));
		} else {
			dlg->set_string("progress_bar", format("%.1f%%", progress * 100));
		}
		//dlg->setString("progress_bar", str);
		dlg->set_float("progress_bar", progress);
		hui::Application::do_single_main_loop();
		allow_next = t + PROGRESS_DT;
	}
}


void Progress::set(float progress) {
	set("", progress);
}

void Progress::set_kaba(const string &str, float progress) {
	set(str, progress);
}

void Progress::cancel() {
	cancelled = true;
	out_cancel.notify();
}

bool Progress::is_cancelled() {
	return cancelled;
}


ProgressCancelable::ProgressCancelable() :
	Progress()
{
}

ProgressCancelable::ProgressCancelable(const string &str, hui::Window *parent) :
	Progress()
{
	if (parent) {
		dlg = hui::create_resource_dialog("progress_cancelable_dialog", parent);
		dlg->set_string("progress_bar", str);
		dlg->set_float("progress_bar", 0);
		dlg->show();
		dlg->event("hui:close", [this]{ cancel(); });
		dlg->event("cancel", [this]{ cancel(); });
		hui::Application::do_single_main_loop();
	}
}


void ProgressCancelable::__init__(const string &str, hui::Window *parent) {
	new(this) ProgressCancelable(str, parent);
}

void ProgressCancelable::__delete__() {
	this->ProgressCancelable::~ProgressCancelable();
}

}
