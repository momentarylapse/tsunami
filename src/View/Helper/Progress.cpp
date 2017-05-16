/*
 * Progress.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "Progress.h"

const string Progress::MESSAGE_CANCEL = "Cancel";

Progress::Progress(const string &str, hui::Window *parent) :
	Observable("Progress")
{
	dlg = NULL;
	if (parent){
		dlg = hui::CreateResourceDialog("progress_dialog", parent);
		dlg->setString("progress_bar", str);
		dlg->setFloat("progress_bar", 0);
		dlg->show();
		dlg->event("hui:close", &hui::FuncIgnore);
		hui::Application::doSingleMainLoop();
	}
	cancelled = false;
}

Progress::Progress() :
	Observable("Progress"), dlg(NULL)
{
	cancelled = false;
}

Progress::~Progress()
{
	if (dlg)
		delete(dlg);
}


void Progress::set(const string &str, float progress)
{
	if (dlg){
		dlg->setString("progress_bar", str);
		dlg->setFloat("progress_bar", progress);
		hui::Application::doSingleMainLoop();
	}
}


void Progress::set(float progress)
{
	if (dlg){
		dlg->setFloat("progress_bar", progress);
		hui::Application::doSingleMainLoop();
	}
}

void Progress::cancel()
{
	cancelled = true;
	notify(MESSAGE_CANCEL);
}

bool Progress::isCancelled()
{
	return cancelled;
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
