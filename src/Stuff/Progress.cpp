/*
 * Progress.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "Progress.h"

static Progress *_progress_;

Progress::Progress() :
	dlg(NULL)
{
	_progress_ = this;
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

void IgnoreEvent(){}

void Progress::Start(const string &str, float progress)
{
	msg_db_r("ProgressStart", 2);
	if (dlg == NULL){
		dlg = HuiCreateResourceDialog("progress_dialog", HuiCurWindow);
		dlg->SetString("progress_bar", str);
		dlg->SetFloat("progress_bar", progress);
		dlg->Update();
		dlg->Event("hui:close", &IgnoreEvent);
		HuiDoSingleMainLoop();
	}
	Cancelled = false;
	msg_db_l(2);
}

void Progress::Cancel()
{
	Cancelled = true;
}

bool Progress::IsCancelled()
{
	return Cancelled;
}

void OnProgressClose()
{	_progress_->Cancel();	}

void Progress::StartCancelable(const string &str, float progress)
{
	msg_db_r("ProgressStart", 2);
	if (dlg == NULL){
		dlg = HuiCreateResourceDialog("progress_cancelable_dialog", HuiCurWindow);
		dlg->SetString("progress_bar", str);
		dlg->SetFloat("progress_bar", progress);
		dlg->Update();
		dlg->Event("hui:close", &OnProgressClose);
		dlg->Event("cancel", &OnProgressClose);
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
