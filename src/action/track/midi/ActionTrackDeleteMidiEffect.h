/*
 * ActionTrackDeleteMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#pragma once

#include "../../../module/midi/MidiEffect.h"
#include <lib/history/Action.h>

namespace tsunami {

class Track;

class ActionTrackDeleteMidiEffect: public history::Action {
public:
	ActionTrackDeleteMidiEffect(Track *t, int index);

	string name() const override { return ":##:delete midi fx"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<MidiEffect> effect;
	Track *track;
	int index;
};

}
