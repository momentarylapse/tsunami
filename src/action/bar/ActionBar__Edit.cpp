/*
 * ActionBar__Edit.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionBar__Edit.h"

#include <assert.h>
#include <algorithm>
#include "../../data/rhythm/Bar.h"
#include "../../data/Track.h"
#include "../../data/Song.h"


ActionBar__Edit::ActionBar__Edit(int _index, int _length, Array<int> &_beats, int _divisor) {
	index = _index;
	length = _length;
	divisor = _divisor;
	beats = _beats;
}

void *ActionBar__Edit::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	auto bar = s->bars[index];

	std::swap(length, bar->length);
	std::swap(beats, bar->beats);
	std::swap(divisor, bar->divisor);
	bar->update_total();
	s->bars._update_offsets();


	s->x_message_data = {index, length, bar->length};
	s->out_scale_bars.notify();

	s->out_edit_bars.notify();

	return nullptr;
}

void ActionBar__Edit::undo(Data *d) {
	execute(d);
}

