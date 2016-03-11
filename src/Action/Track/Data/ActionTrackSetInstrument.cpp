/*
 * ActionTrackSetInstrument.cpp
 *
 *  Created on: Dec 2, 2015
 *      Author: ankele
 */

#include "ActionTrackSetInstrument.h"
#include "../../../Data/Track.h"

ActionTrackSetInstrument::ActionTrackSetInstrument(Track* t, const Instrument &instrument)
{
	track_no = t->get_index();
	old_value = t->instrument;
	new_value = instrument;
}

void* ActionTrackSetInstrument::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->instrument = new_value;
	t->midi.clear_meta();

	t->notify();

	return NULL;
}

void ActionTrackSetInstrument::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->instrument = old_value;
	t->midi.clear_meta();

	t->notify();
}

bool ActionTrackSetInstrument::mergable(Action* a)
{
	ActionTrackSetInstrument *aa = dynamic_cast<ActionTrackSetInstrument*>(a);
	if (!aa)
		return false;
	return (aa->track_no == track_no);
}

bool ActionTrackSetInstrument::absorb(ActionMergableBase* a)
{
	if (!mergable(a))
		return false;
	ActionTrackSetInstrument* aa = dynamic_cast<ActionTrackSetInstrument*>(a);
	new_value = aa->new_value;
	return true;
}
