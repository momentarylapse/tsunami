/*
 * ActionTrackSetInstrument.cpp
 *
 *  Created on: Dec 2, 2015
 *      Author: ankele
 */

#include "ActionTrackSetInstrument.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"

ActionTrackSetInstrument::ActionTrackSetInstrument(Track* t, const Instrument &instrument) {
	track = t;
	old_value = t->instrument;
	new_value = instrument;

	for (auto l: weak(track->layers))
		for (auto n: weak(l->midi)) {
			StringChange c;
			c.note = n;
			c.from = n->stringno;
			c.to = instrument.make_string_valid(n->pitch, n->stringno);
			if (c.from != c.to)
				string_change.add(c);
		}

}

void* ActionTrackSetInstrument::execute(Data* d) {
	track->instrument = new_value;
	for (auto *l: weak(track->layers))
		l->midi.reset_clef();

	for (auto &c: string_change)
		c.note->stringno = c.to;

	track->notify();

	return nullptr;
}

void ActionTrackSetInstrument::undo(Data* d) {
	track->instrument = old_value;
	for (auto l: track->layers)
		l->midi.reset_clef();

	for (auto &c: string_change)
		c.note->stringno = c.from;

	track->notify();
}

bool ActionTrackSetInstrument::mergable(Action* a) {
	auto *aa = dynamic_cast<ActionTrackSetInstrument*>(a);
	if (!aa)
		return false;
	return (aa->track == track);
}

bool ActionTrackSetInstrument::absorb(ActionMergableBase* a) {
	if (!mergable(a))
		return false;
	auto* aa = dynamic_cast<ActionTrackSetInstrument*>(a);
	new_value = aa->new_value;
	string_change.append(aa->string_change);
	return true;
}
