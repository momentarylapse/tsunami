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
}

Progress::~Progress()
{
}


void Progress::Set(const string &str, float progress)
{
	msg_db_r("Progress.Set", 2);
	if (dlg){
		dlg->SetString("progress_bar", str);
		dlg->SetFloat("progress_bar", progress);
		HuiDoSingleMainLoop();
	}
	msg_db_l(2);
}


void Progress::Set(float progress)
{
	msg_db_r("Progress.Set", 2);
	if (dlg){
		dlg->SetFloat("progress_bar", progress);
		HuiDoSingleMainLoop();
	}
	msg_db_l(2);
}

void Progress::Start(const string &str, float progress)
{
	msg_db_r("ProgressStart", 2);
	if (dlg == NULL){
		dlg = HuiCreateResourceDialog("progress_dialog", HuiCurWindow);
		dlg->SetString("progress_bar", str);
		dlg->SetFloat("progress_bar", progress);
		dlg->Show();
		dlg->Event("hui:close", &HuiFuncIgnore);
		HuiDoSingleMainLoop();
	}
	Cancelled = false;
	msg_db_l(2);
}

void Progress::Cancel()
{
	Notify(MESSAGE_CANCEL);
	Cancelled = true;
}

bool Progress::IsCancelled()
{
	return Cancelled;
}

void Progress::StartCancelable(const string &str, float progress)
{
	msg_db_r("ProgressStart", 2);
	if (dlg == NULL){
		dlg = HuiCreateResourceDialog("progress_cancelable_dialog", HuiCurWindow);
		dlg->SetString("progress_bar", str);
		dlg->SetFloat("progress_bar", progress);
		dlg->Show();
		dlg->EventM("hui:close", this, &Progress::Cancel);
		dlg->EventM("cancel", this, &Progress::Cancel);
		HuiDoSingleMainLoop();
	}
	Cancelled = false;
	msg_db_l(2);
}

void Progress::End()
{
	msg_db_r("ProgressEnd", 2);
	if (dlg){
		delete(dlg);
		dlg = NULL;
	}
	msg_db_l(2);
}
