/*
 * Progress.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "Progress.h"

const string Progress::MESSAGE_CANCEL = "Cancel";

Progress::Progress() :
	Observable("Progress"), dlg(NULL)
{
	Cancelled = false;
}

Progress::~Progress()
{
}


void Progress::set(const string &str, float progress)
{
	if (dlg){
		dlg->setString("progress_bar", str);
		dlg->setFloat("progress_bar", progress);
		HuiDoSingleMainLoop();
	}
}


void Progress::set(float progress)
{
	if (dlg){
		dlg->setFloat("progress_bar", progress);
		HuiDoSingleMainLoop();
	}
}

void Progress::start(const string &str, float progress)
{
	if ((dlg == NULL) && (HuiCurWindow)){
		dlg = HuiCreateResourceDialog("progress_dialog", HuiCurWindow);
		dlg->setString("progress_bar", str);
		dlg->setFloat("progress_bar", progress);
		dlg->show();
		dlg->eventS("hui:close", &HuiFuncIgnore);
		HuiDoSingleMainLoop();
	}
	Cancelled = false;
}

void Progress::cancel()
{
	notify(MESSAGE_CANCEL);
	Cancelled = true;
}

bool Progress::isCancelled()
{
	return Cancelled;
}

void Progress::startCancelable(const string &str, float progress)
{
	if ((dlg == NULL) && (HuiCurWindow)){
		dlg = HuiCreateResourceDialog("progress_cancelable_dialog", HuiCurWindow);
		dlg->setString("progress_bar", str);
		dlg->setFloat("progress_bar", progress);
		dlg->show();
		dlg->event("hui:close", this, &Progress::cancel);
		dlg->event("cancel", this, &Progress::cancel);
		HuiDoSingleMainLoop();
	}
	Cancelled = false;
}

void Progress::end()
{
	if (dlg){
		delete(dlg);
		dlg = NULL;
	}
}
