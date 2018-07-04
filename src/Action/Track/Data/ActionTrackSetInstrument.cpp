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
	track = t;
	old_value = t->instrument;
	new_value = instrument;
}

void* ActionTrackSetInstrument::execute(Data* d)
{
	track->instrument = new_value;
	for (TrackLayer *l: track->layers)
		l->midi.clear_meta();

	track->notify();

	return NULL;
}

void ActionTrackSetInstrument::undo(Data* d)
{
	track->instrument = old_value;
	for (TrackLayer *l: track->layers)
		l->midi.clear_meta();

	track->notify();
}

bool ActionTrackSetInstrument::mergable(Action* a)
{
	ActionTrackSetInstrument *aa = dynamic_cast<ActionTrackSetInstrument*>(a);
	if (!aa)
		return false;
	return (aa->track == track);
}

bool ActionTrackSetInstrument::absorb(ActionMergableBase* a)
{
	if (!mergable(a))
		return false;
	ActionTrackSetInstrument* aa = dynamic_cast<ActionTrackSetInstrument*>(a);
	new_value = aa->new_value;
	return true;
}
