/*
 * BarAddDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarAddDialog.h"
#include "../../Data/Song.h"
#include "../AudioView.h"

BarAddDialog::BarAddDialog(HuiWindow *root, Song *s, AudioView *v):
	HuiDialog("", 100, 100, root, false)
{
	fromResource("bar_add_dialog");
	song = s;
	view = v;
	bars = view->sel.bars;

	// no reference bar selected -> use last bar
	int ref = bars.start();
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

	check("insert:after", true);

	event("ok", this, &BarAddDialog::onOk);
	event("cancel", this, &BarAddDialog::onClose);
	event("hui:close", this, &BarAddDialog::onClose);
}

void BarAddDialog::onOk()
{
	int count = getInt("count");
	int beats = getInt("beats");
	float bpm = getFloat("bpm");
	bool after = isChecked("insert:after");
	song->action_manager->beginActionGroup();

	if (!song->getTimeTrack())
		song->addTrack(Track::TYPE_TIME, 0);

	int index = 0;
	if (after){
		if (bars.length > 0)
			index = bars.end();
	}else{
		if (bars.length > 0)
			index = bars.start();
	}

	for (int i=0; i<count; i++)
		song->addBar(index, bpm, beats, view->bars_edit_data);
	song->action_manager->endActionGroup();

	if (!after){
		view->sel_raw.offset += song->bars[index].length * count;
		view->updateSelection();
	}

	delete(this);
}

void BarAddDialog::onClose()
{
	delete(this);
}
