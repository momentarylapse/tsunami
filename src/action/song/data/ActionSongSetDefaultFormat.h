/*
 * ActionSongSetDefaultFormat.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
#include "../../../data/audio/AudioBuffer.h"


class ActionSongSetDefaultFormat : public Action {
public:
	ActionSongSetDefaultFormat(SampleFormat format, int compression);

	string name() const override { return ":##:set default format"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	SampleFormat format;
	int compression;
};
