/*
 * ActionTrackAddAudioEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

struct Track;
class AudioEffect;

class ActionTrackAddAudioEffect: public history::Action {
public:
	ActionTrackAddAudioEffect(Track *t, shared<AudioEffect> effect);

	string name() const override { return ":##:add fx"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<AudioEffect> effect;
	Track *track;
};

}
