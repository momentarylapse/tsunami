/*
 * ActionSongSetSampleRate.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#pragma once

#include "../../ActionMergable.h"

class Song;

class ActionSongSetSampleRate : public ActionMergable<int> {
public:
	ActionSongSetSampleRate(Song *s, int sample_rate);

	string name() const override { return ":##:set samplerate"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;
};
