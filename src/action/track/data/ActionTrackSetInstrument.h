/*
 * ActionTrackSetInstrument.h
 *
 *  Created on: Dec 2, 2015
 *      Author: ankele
 */

#pragma once

#include <lib/history/MergableAction.h>
#include "../../../data/midi/Instrument.h"

namespace tsunami {

struct Track;
struct MidiNote;

class ActionTrackSetInstrument: public history::MergableValueAction<Instrument> {
public:
	ActionTrackSetInstrument(Track *t, const Instrument &instrument);

	string name() const override { return ":##:set instrument"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;
	bool absorb(Action *a) override;

private:
	Track *track;

	struct StringChange {
		MidiNote* note;
		int from, to;
	};
	Array<StringChange> string_change;
};

}
