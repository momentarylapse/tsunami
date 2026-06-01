/*
 * ActionTrackSetAudioEffectEnabledWetness.h
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class AudioEffect;

class ActionTrackSetAudioEffectEnabledWetness: public history::Action {
public:
	ActionTrackSetAudioEffectEnabledWetness(AudioEffect *fx, bool enabled, float wetness);

	string name() const override { return ":##:enable fx"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	AudioEffect *fx;
	bool enabled;
	float wetness;
};

}
