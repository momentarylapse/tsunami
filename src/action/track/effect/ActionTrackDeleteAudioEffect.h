/*
 * ActionTrackDeleteEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class Track;
class AudioEffect;

class ActionTrackDeleteEffect: public Action {
public:
	ActionTrackDeleteEffect(Track *t, int index);

	string name() const override { return ":##:delete fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<AudioEffect> effect;
	Track *track;
	int index;
};

}
