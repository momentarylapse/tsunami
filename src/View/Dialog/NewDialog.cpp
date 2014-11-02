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

	setString("sample_rate", "22050");
	setString("sample_rate", i2s(DEFAULT_SAMPLE_RATE));
	setString("sample_rate", "48000");
	setString("sample_rate", "96000");
	setInt("sample_rate", 1);
	hideControl("nd_g_metronome_params", true);

	event("cancel", this, &NewDialog::onClose);
	event("hui:close", this, &NewDialog::onClose);
	event("ok", this, &NewDialog::onOk);
	event("metronome", this, &NewDialog::onMetronome);
}

NewDialog::~NewDialog()
{
}

void NewDialog::onClose()
{
	delete(this);
}

void NewDialog::onOk()
{
	int sample_rate = getString("sample_rate")._int();
	audio->newWithOneTrack(Track::TYPE_AUDIO, sample_rate);
	audio->action_manager->enable(false);
	if (isChecked("metronome")){
		Track *t = audio->addTrack(Track::TYPE_TIME, 0);
		t->addBars(-1, getFloat("beats_per_minute"), getInt("beats_per_bar"), getInt("num_bars"));
	}
	audio->action_manager->enable(true);
	onClose();
}

void NewDialog::onMetronome()
{
	hideControl("nd_g_metronome_params", !isChecked(""));
}

