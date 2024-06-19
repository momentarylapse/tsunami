/*
 * ActionSongSetSampleRate.cpp
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#include "ActionSongSetSampleRate.h"
#include "../../../data/Song.h"

namespace tsunami {

ActionSongSetSampleRate::ActionSongSetSampleRate(Song *s, int _sample_rate) {
	new_value = _sample_rate;
	old_value = s->sample_rate;
}

void *ActionSongSetSampleRate::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	s->sample_rate = new_value;

	return nullptr;
}

void ActionSongSetSampleRate::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	s->sample_rate = old_value;
}


bool ActionSongSetSampleRate::mergable(Action *a) {
	ActionSongSetSampleRate *aa = dynamic_cast<ActionSongSetSampleRate*>(a);
	return aa;
}

}
