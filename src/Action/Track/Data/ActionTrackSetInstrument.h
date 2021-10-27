/*
 * ActionTrackSetInstrument.h
 *
 *  Created on: Dec 2, 2015
 *      Author: ankele
 */

#pragma once

#include "../../ActionMergable.h"
#include "../../../Data/Midi/Instrument.h"
class Track;
class MidiNote;

class ActionTrackSetInstrument: public ActionMergable<Instrument> {
public:
	ActionTrackSetInstrument(Track *t, const Instrument &instrument);

	string name() const override { return ":##:set instrument"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;
	bool absorb(ActionMergableBase *a) override;

private:
	Track *track;

	struct StringChange {
		MidiNote* note;
		int from, to;
	};
	Array<StringChange> string_change;
};
