/*
 * Progress.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "Progress.h"

const string Progress::MESSAGE_CANCEL = "Cancel";

Progress::Progress(const string &str, HuiWindow *parent) :
	Observable("Progress")
{
	dlg = HuiCreateResourceDialog("progress_dialog", parent);
	dlg->setString("progress_bar", str);
	dlg->setFloat("progress_bar", 0);
	dlg->show();
	dlg->eventS("hui:close", &HuiFuncIgnore);
	HuiDoSingleMainLoop();
	cancelled = false;
}

Progress::Progress() :
	Observable("Progress"), dlg(NULL)
{
	cancelled = false;
}

Progress::~Progress()
{
	delete(dlg);
}


void Progress::set(const string &str, float progress)
{
	dlg->setString("progress_bar", str);
	dlg->setFloat("progress_bar", progress);
	HuiDoSingleMainLoop();
}


void Progress::set(float progress)
{
	dlg->setFloat("progress_bar", progress);
	HuiDoSingleMainLoop();
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



ProgressCancelable::ProgressCancelable(const string &str, HuiWindow *parent) :
	Progress()
{
	dlg = HuiCreateResourceDialog("progress_cancelable_dialog", parent);
	dlg->setString("progress_bar", str);
	dlg->setFloat("progress_bar", 0);
	dlg->show();
	dlg->event("hui:close", this, &Progress::cancel);
	dlg->event("cancel", this, &Progress::cancel);
	HuiDoSingleMainLoop();
}

ProgressCancelable::~ProgressCancelable()
{
}
