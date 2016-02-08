/*
 * BarAddDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarAddDialog.h"
#include "../../Data/Song.h"

BarAddDialog::BarAddDialog(HuiWindow *root, Song *_s, int _index, bool _apply_to_midi):
	HuiDialog("", 100, 100, root, false)
{
	fromResource("bar_add_dialog");
	song = _s;
	index = _index;
	apply_to_midi = _apply_to_midi;

	// no reference bar selected -> use last bar
	int ref = index;
	if (ref < 0)
		ref = song->bars.num;

	setInt("count", 1);
	int beats = 4;
	float bpm = 90.0f;

	// get default data from "selected" reference bar
	if (song->bars.num > 0){
		foreachi(BarPattern &b, song->bars, i)
			if ((i <= ref) and (b.num_beats > 0)){
				beats = b.num_beats;
				bpm = song->sample_rate * 60.0f / (b.length / b.num_beats);
			}
	}
	setInt("beats", beats);
	setFloat("bpm", bpm);

	event("ok", this, &BarAddDialog::onOk);
	event("cancel", this, &BarAddDialog::onClose);
	event("hui:close", this, &BarAddDialog::onClose);
}

void BarAddDialog::onOk()
{
	int count = getInt("count");
	int beats = getInt("beats");
	float bpm = getFloat("bpm");
	song->action_manager->beginActionGroup();

	for (int i=0; i<count; i++)
		song->addBar(index, bpm, beats, apply_to_midi);
	song->action_manager->endActionGroup();

	delete(this);
}

void BarAddDialog::onClose()
{
	delete(this);
}
