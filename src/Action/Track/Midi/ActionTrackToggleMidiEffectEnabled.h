/*
 * ActionTrackToggleMidiEffectEnabled.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
class MidiEffect;

class ActionTrackToggleMidiEffectEnabled: public Action {
public:
	ActionTrackToggleMidiEffectEnabled(MidiEffect *fx);

	string name() const override { return ":##:enable midi fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	MidiEffect *fx;
};
