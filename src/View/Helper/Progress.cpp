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
		dlg->SetString("progress_bar", str);
		dlg->SetFloat("progress_bar", progress);
		HuiDoSingleMainLoop();
	}
}


void Progress::set(float progress)
{
	if (dlg){
		dlg->SetFloat("progress_bar", progress);
		HuiDoSingleMainLoop();
	}
}

void Progress::start(const string &str, float progress)
{
	if ((dlg == NULL) && (HuiCurWindow)){
		dlg = HuiCreateResourceDialog("progress_dialog", HuiCurWindow);
		dlg->SetString("progress_bar", str);
		dlg->SetFloat("progress_bar", progress);
		dlg->Show();
		dlg->Event("hui:close", &HuiFuncIgnore);
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
		dlg->SetString("progress_bar", str);
		dlg->SetFloat("progress_bar", progress);
		dlg->Show();
		dlg->EventM("hui:close", this, &Progress::cancel);
		dlg->EventM("cancel", this, &Progress::cancel);
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
