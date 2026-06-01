/*
 * ActionTrackMoveAudioEffect.h
 *
 *  Created on: 20.11.2018
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class AudioEffect;
class Track;

class ActionTrackMoveAudioEffect: public history::Action {
public:
	ActionTrackMoveAudioEffect(Track *track, int source, int target);

	string name() const override { return ":##:move fx"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	Track *track;
	int source;
	int target;
};

}
