/*
 * NewDialog.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "NewDialog.h"

NewDialog::NewDialog(CHuiWindow *_parent, bool _allow_parent, AudioFile *a):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	audio = a;

	// dialog
	FromResource("new_dialog");

	SetInt("sample_rate", DEFAULT_SAMPLE_RATE);

	EventM("cancel", this, (void(HuiEventHandler::*)())&NewDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&NewDialog::OnClose);
	EventM("ok", this, (void(HuiEventHandler::*)())&NewDialog::OnOk);
}

NewDialog::~NewDialog()
{
	// TODO Auto-generated destructor stub
}

void NewDialog::OnClose()
{
	delete(this);
}

void NewDialog::OnOk()
{
	int sample_rate = GetInt("sample_rate");
	audio->NewWithOneTrack(sample_rate);
	OnClose();
}

