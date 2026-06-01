/*
 * ActionSongSetSampleRate.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>

namespace tsunami {

class Song;

class ActionSongSetSampleRate : public history::MergableValueAction<int> {
public:
	ActionSongSetSampleRate(Song *s, int sample_rate);

	string name() const override { return ":##:set samplerate"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;
};

}
