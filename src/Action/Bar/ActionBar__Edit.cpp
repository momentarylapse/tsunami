/*
 * ActionBar__Edit.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionBar__Edit.h"

#include <assert.h>
#include <algorithm>
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Track.h"
#include "../../Data/Song.h"


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
	s->notify(s->MESSAGE_SCALE_BARS);

	s->notify(s->MESSAGE_EDIT_BARS);

	return nullptr;
}

void ActionBar__Edit::undo(Data *d) {
	execute(d);
}

