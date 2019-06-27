/*
 * Action__AddBar.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionBar__Add.h"
#include "../../Data/Song.h"
#include "../../Data/Rhythm/Bar.h"
#include <assert.h>


ActionBar__Add::ActionBar__Add(int _index, Bar *_bar) {
	index = _index;
	bar = _bar;
}

ActionBar__Add::~ActionBar__Add() {
	if (bar)
		delete bar;
}

void *ActionBar__Add::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index <= s->bars.num);

	s->bars.insert(bar, index);
	s->notify(s->MESSAGE_EDIT_BARS);
	bar = nullptr;

	return nullptr;
}

void ActionBar__Add::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	bar = s->bars[index];
	s->bars.erase(index);
	s->notify(s->MESSAGE_EDIT_BARS);
}

