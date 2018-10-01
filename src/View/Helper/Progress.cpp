/*
 * Progress.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "Progress.h"

const string Progress::MESSAGE_CANCEL = "Cancel";

const float PROGRESS_DT = 0.05f;

Progress::Progress(const string &str, hui::Window *parent)
{
	dlg = nullptr;
	allow_next = 0;
	if (parent){
		dlg = hui::CreateResourceDialog("progress_dialog", parent);
		dlg->event("hui:close", &hui::FuncIgnore);
		set(str, 0);
		dlg->show();
	}
	cancelled = false;
}

Progress::Progress()
{
	dlg = nullptr;
	cancelled = false;
	allow_next = 0;
}

Progress::~Progress()
{
	if (dlg)
		delete(dlg);
}



void Progress::__init__(const string &str, hui::Window *parent)
{
	new(this) Progress(str, parent);
}

void Progress::__delete__()
{
	this->~Progress();
}


void Progress::set(const string &str, float progress)
{
	if (dlg){
		float t = timer.peek();
		if (t < allow_next)
			return;
		if (str.num > 0)
			dlg->setTitle(str);
		if (t > 2){
			int eta = (int)(t / progress * (1-progress) + 0.7f);
			if (eta >= 60)
				dlg->setString("progress_bar", format("%.1f%% (%dmin %ds)", progress * 100, eta/60, eta %60));
			else
				dlg->setString("progress_bar", format("%.1f%% (%ds)", progress * 100, eta));
		}else{
			dlg->setString("progress_bar", format("%.1f%%", progress * 100));
		}
		//dlg->setString("progress_bar", str);
		dlg->setFloat("progress_bar", progress);
		hui::Application::doSingleMainLoop();
		allow_next = t + PROGRESS_DT;
	}
}


void Progress::set(float progress)
{
	set("", progress);
}

void Progress::set_kaba(const string &str, float progress)
{
	set(str, progress);
}

void Progress::cancel()
{
	cancelled = true;
	notify(MESSAGE_CANCEL);
}

bool Progress::is_cancelled()
{
	return cancelled;
}


ProgressCancelable::ProgressCancelable() :
	Progress()
{
}

ProgressCancelable::ProgressCancelable(const string &str, hui::Window *parent) :
	Progress()
{
	if (parent){
		dlg = hui::CreateResourceDialog("progress_cancelable_dialog", parent);
		dlg->setString("progress_bar", str);
		dlg->setFloat("progress_bar", 0);
		dlg->show();
		dlg->event("hui:close", std::bind(&Progress::cancel, this));
		dlg->event("cancel", std::bind(&Progress::cancel, this));
		hui::Application::doSingleMainLoop();
	}
}

ProgressCancelable::~ProgressCancelable()
{
}


void ProgressCancelable::__init__(const string &str, hui::Window *parent)
{
	new(this) ProgressCancelable(str, parent);
}

void ProgressCancelable::__delete__()
{
	this->ProgressCancelable::~ProgressCancelable();
}
