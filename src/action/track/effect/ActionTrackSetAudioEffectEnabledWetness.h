/*
 * ActionTrackSetAudioEffectEnabledWetness.h
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class AudioEffect;

class ActionTrackSetAudioEffectEnabledWetness: public Action {
public:
	ActionTrackSetAudioEffectEnabledWetness(AudioEffect *fx, bool enabled, float wetness);

	string name() const override { return ":##:enable fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	AudioEffect *fx;
	bool enabled;
	float wetness;
};

}
