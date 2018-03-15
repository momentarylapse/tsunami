/*
 * BarAddDialog.cpp
 *
 *  Created on: 30.10.2015
 *      Author: michi
 */

#include "BarAddDialog.h"
#include "../../Data/Song.h"
#include "../../Rhythm/Bar.h"
#include "../AudioView.h"

BarAddDialog::BarAddDialog(hui::Window *root, Song *s, AudioView *v, int _index):
	hui::Dialog("", 100, 100, root, false)
{
	fromResource("bar_add_dialog");
	song = s;
	view = v;
	index = max(_index, 0);


	setInt("count", 1);
	int beats = 4;
	int sub_beats = 1;
	float bpm = 90.0f;

	// get default data from "selected" reference bar
	if (song->bars.num > 0){
		foreachi(Bar *b, song->bars, i)
			if ((i <= index) and (b->num_beats > 0)){
				beats = b->num_beats;
				sub_beats = b->num_sub_beats;
				bpm = song->sample_rate * 60.0f / (b->length / b->num_beats);
			}
	}
	setInt("beats", beats);
	setInt("sub_beats", sub_beats);
	setFloat("bpm", bpm);

	event("ok", std::bind(&BarAddDialog::onOk, this));
	event("cancel", std::bind(&BarAddDialog::onClose, this));
	event("hui:close", std::bind(&BarAddDialog::onClose, this));
}

void BarAddDialog::onOk()
{
	int count = getInt("count");
	int beats = getInt("beats");
	int sub_beats = getInt("sub_beats");
	float bpm = getFloat("bpm");
	song->action_manager->beginActionGroup();

	if (!song->getTimeTrack())
		song->addTrack(Track::TYPE_TIME, 0);

	for (int i=0; i<count; i++)
		song->addBar(index, bpm, beats, sub_beats, view->bars_edit_data);
	song->action_manager->endActionGroup();

	destroy();
}

void BarAddDialog::onClose()
{
	destroy();
}
