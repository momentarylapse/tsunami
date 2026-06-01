/*
 * ActionTrackAddMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class Track;
class MidiEffect;

class ActionTrackAddMidiEffect: public history::Action {
public:
	ActionTrackAddMidiEffect(Track *t, shared<MidiEffect> effect);

	string name() const override { return ":##:add midi fx"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<MidiEffect> effect;
	Track *track;
};

}
