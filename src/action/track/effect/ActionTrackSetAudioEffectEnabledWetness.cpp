/*
 * ActionTrackSetAudioEffectEnabledWetness.cpp
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#include "ActionTrackSetAudioEffectEnabledWetness.h"
#include "../../../module/audio/AudioEffect.h"
#include "../../../data/Song.h"

ActionTrackSetAudioEffectEnabledWetness::ActionTrackSetAudioEffectEnabledWetness(AudioEffect *_fx, bool _enabled, float _wetness) {
	fx = _fx;
	enabled = _enabled;
	wetness = _wetness;
}

void *ActionTrackSetAudioEffectEnabledWetness::execute(Data *d) {
	std::swap(fx->enabled, enabled);
	std::swap(fx->wetness, wetness);
	fx->out_changed.notify();
	((Song*)d)->out_enable_fx.notify();

	return nullptr;
}

void ActionTrackSetAudioEffectEnabledWetness::undo(Data *d) {
	execute(d);
}


