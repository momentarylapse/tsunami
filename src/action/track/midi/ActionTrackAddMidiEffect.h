/*
 * ActionTrackAddMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class Track;
class MidiEffect;

class ActionTrackAddMidiEffect: public Action {
public:
	ActionTrackAddMidiEffect(Track *t, shared<MidiEffect> effect);

	string name() const override { return ":##:add midi fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<MidiEffect> effect;
	Track *track;
};

}
