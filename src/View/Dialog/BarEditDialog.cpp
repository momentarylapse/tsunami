/*
 * BarEditDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarEditDialog.h"
#include "../../Data/Song.h"

BarEditDialog::BarEditDialog(HuiWindow *root, Song *_song, const Range &_bars, bool _apply_to_midi):
	HuiDialog("", 100, 100, root, false)
{
	fromResource("bar_edit_dialog");
	song = _song;
	for (int i=_bars.start(); i<_bars.end(); i++)
		sel.add(i);
	apply_to_midi = _apply_to_midi;

	BarPattern &b = song->bars[sel[0]];
	setInt("beats", b.num_beats);
	setFloat("bpm", song->sample_rate * 60.0f / (b.length / b.num_beats));
	//check("edit_bpm", true);

	event("ok", this, &BarEditDialog::onOk);
	event("cancel", this, &BarEditDialog::onClose);
	event("hui:close", this, &BarEditDialog::onClose);
	event("beats", this, &BarEditDialog::onBeats);
	event("bpm", this, &BarEditDialog::onBpm);
}

void BarEditDialog::onOk()
{
	int beats = getInt("beats");
	float bpm = getFloat("bpm");
	bool edit_beats = isChecked("edit_beats");
	bool edit_bpm = isChecked("edit_bpm");
	song->action_manager->beginActionGroup();
	foreachb(int i, sel){
		BarPattern b = song->bars[i];
		if (edit_beats)
			b.num_beats = beats;
		if (edit_bpm)
			b.length = song->sample_rate * 60.0f * b.num_beats / bpm;
		song->editBar(i, b, apply_to_midi);
	}
	song->action_manager->endActionGroup();

	delete(this);
}

void BarEditDialog::onClose()
{
	delete(this);
}

void BarEditDialog::onBeats()
{
	check("edit_beats", true);
}

void BarEditDialog::onBpm()
{
	check("edit_bpm", true);
}
