/*
 * NewDialog.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "NewDialog.h"

NewDialog::NewDialog(HuiWindow *_parent, Song *s):
	HuiWindow("new_dialog", _parent, false)
{
	song = s;

	addString("sample_rate", "22050");
	addString("sample_rate", i2s(DEFAULT_SAMPLE_RATE));
	addString("sample_rate", "48000");
	addString("sample_rate", "96000");
	setInt("sample_rate", 1);
	hideControl("nd_g_metronome_params", true);

	event("cancel", this, &NewDialog::onClose);
	event("hui:close", this, &NewDialog::onClose);
	event("ok", this, &NewDialog::onOk);
	event("metronome", this, &NewDialog::onMetronome);
	event("new_track_type:midi", this, &NewDialog::onTypeMidi);
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
	bool midi = isChecked("new_track_type:midi");
	song->newWithOneTrack(midi ? Track::TYPE_MIDI : Track::TYPE_AUDIO, sample_rate);
	song->action_manager->enable(false);
	if (isChecked("metronome")){
		Track *t = song->addTrack(Track::TYPE_TIME, 0);
		int count = getInt("num_bars");
		for (int i=0; i<count; i++)
			song->addBar(-1, getFloat("beats_per_minute"), getInt("beats_per_bar"), false);
	}
	song->action_manager->enable(true);
	song->notify(song->MESSAGE_NEW);
	onClose();
}

void NewDialog::onMetronome()
{
	hideControl("nd_g_metronome_params", !isChecked(""));
}

void NewDialog::onTypeMidi()
{
	//hideControl("nd_g_metronome_params", !isChecked(""));
}

