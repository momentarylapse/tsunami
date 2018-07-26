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


ActionBar__Edit::ActionBar__Edit(int _index, int _length, int _num_beats, int _num_sub_beats)
{
	index = _index;
	length = _length;
	num_beats = _num_beats;
	num_sub_beats = _num_sub_beats;
}

void *ActionBar__Edit::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	Bar *bar = s->bars[index];

	std::swap(length, bar->length);
	std::swap(num_beats, bar->num_beats);
	std::swap(num_sub_beats, bar->num_sub_beats);

	s->notify(s->MESSAGE_EDIT_BARS);

	return nullptr;
}

void ActionBar__Edit::undo(Data *d)
{
	execute(d);
}

