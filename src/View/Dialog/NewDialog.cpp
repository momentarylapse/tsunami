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

	SetString("sample_rate", "22050");
	SetString("sample_rate", i2s(DEFAULT_SAMPLE_RATE));
	SetString("sample_rate", "48000");
	SetString("sample_rate", "96000");
	SetInt("sample_rate", 1);
	HideControl("nd_g_metronome_params", true);

	EventM("cancel", this, &NewDialog::OnClose);
	EventM("hui:close", this, &NewDialog::OnClose);
	EventM("ok", this, &NewDialog::OnOk);
	EventM("metronome", this, &NewDialog::OnMetronome);
}

NewDialog::~NewDialog()
{
}

void NewDialog::OnClose()
{
	delete(this);
}

void NewDialog::OnOk()
{
	int sample_rate = GetString("sample_rate")._int();
	audio->NewWithOneTrack(Track::TYPE_AUDIO, sample_rate);
	audio->action_manager->Enable(false);
	if (IsChecked("metronome")){
		Track *t = audio->AddTrack(Track::TYPE_TIME, 0);
		t->AddBars(-1, GetFloat("beats_per_minute"), GetInt("beats_per_bar"), GetInt("num_bars"));
	}
	audio->action_manager->Enable(true);
	OnClose();
}

void NewDialog::OnMetronome()
{
	HideControl("nd_g_metronome_params", !IsChecked(""));
}

