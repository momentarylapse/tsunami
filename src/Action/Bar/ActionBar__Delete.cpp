/*
 * ActionBar__Delete.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include <assert.h>
#include "ActionBar__Delete.h"
#include "../../Data/Song.h"
#include "../../Data/Rhythm/Bar.h"


ActionBar__Delete::ActionBar__Delete(int _index) {
	index = _index;
	bar = nullptr;
}

ActionBar__Delete::~ActionBar__Delete() {
	if (bar)
		delete bar;
}

void *ActionBar__Delete::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	bar = s->bars[index];
	s->bars.erase(index);
	s->notify(s->MESSAGE_EDIT_BARS);

	return nullptr;
}

void ActionBar__Delete::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index <= s->bars.num);

	s->bars.insert(bar, index);
	s->notify(s->MESSAGE_EDIT_BARS);
	bar = nullptr;
}
