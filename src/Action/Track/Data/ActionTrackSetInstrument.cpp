/*
 * ActionTrackSetInstrument.cpp
 *
 *  Created on: Dec 2, 2015
 *      Author: ankele
 */

#include "ActionTrackSetInstrument.h"
#include "../../../Data/Track.h"

ActionTrackSetInstrument::ActionTrackSetInstrument(Track* t, const string &_instrument, const Array<int>& _tuning)
{
	track_no = t->get_index();
	instrument = _instrument;
	tuning = _tuning;
}

void* ActionTrackSetInstrument::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	string t1 = t->instrument;
	t->instrument = instrument;
	instrument = t1;

	Array<int> t2 = t->tuning;
	t->tuning = tuning;
	tuning = t2;

	t->notify();

	return NULL;
}

void ActionTrackSetInstrument::undo(Data* d)
{
	execute(d);
}
