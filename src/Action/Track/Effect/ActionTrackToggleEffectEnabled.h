/*
 * ActionTrackToggleEffectEnabled.h
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
class AudioEffect;

class ActionTrackToggleEffectEnabled: public Action {
public:
	ActionTrackToggleEffectEnabled(AudioEffect *fx);

	string name() const override { return ":##:enable fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	AudioEffect *fx;
};
