/*
 * ActionTrackAddAudioEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
class Track;
class AudioEffect;

class ActionTrackAddAudioEffect: public Action {
public:
	ActionTrackAddAudioEffect(Track *t, shared<AudioEffect> effect);

	string name() const override { return ":##:add fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<AudioEffect> effect;
	Track *track;
};
