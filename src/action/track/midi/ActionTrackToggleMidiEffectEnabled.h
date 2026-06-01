/*
 * ActionTrackToggleMidiEffectEnabled.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class MidiEffect;

class ActionTrackToggleMidiEffectEnabled: public history::Action {
public:
	ActionTrackToggleMidiEffectEnabled(MidiEffect *fx);

	string name() const override { return ":##:enable midi fx"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	MidiEffect *fx;
};

}
