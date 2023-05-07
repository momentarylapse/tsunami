/*
 * ActionBar__Delete.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include <assert.h>
#include "ActionBar__Delete.h"
#include "../../data/Song.h"
#include "../../data/rhythm/Bar.h"


ActionBar__Delete::ActionBar__Delete(int _index) {
	index = _index;
	bar = nullptr;
}

void *ActionBar__Delete::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	bar = s->bars[index];
	s->bars.erase(index);
	s->bars._update_offsets();
	s->out_edit_bars.notify();

	return nullptr;
}

void ActionBar__Delete::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index <= s->bars.num);

	s->bars.insert(bar, index);
	s->bars._update_offsets();
	s->out_edit_bars.notify();
}
