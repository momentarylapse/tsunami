/*
 * NewDialog.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "NewDialog.h"

NewDialog::NewDialog(HuiWindow *_parent, bool _allow_parent, AudioFile *a):
	HuiWindow("new_dialog", _parent, _allow_parent)
{
	audio = a;

	SetInt("sample_rate", DEFAULT_SAMPLE_RATE);

	EventM("cancel", this, &NewDialog::OnClose);
	EventM("hui:close", this, &NewDialog::OnClose);
	EventM("ok", this, &NewDialog::OnOk);
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
	audio->NewWithOneTrack(Track::TYPE_AUDIO, sample_rate);
	OnClose();
}

