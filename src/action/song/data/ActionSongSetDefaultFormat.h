/*
 * ActionSongSetDefaultFormat.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../../../data/audio/AudioBuffer.h"

namespace tsunami {

class ActionSongSetDefaultFormat : public history::Action {
public:
	ActionSongSetDefaultFormat(SampleFormat format, int compression);

	string name() const override { return ":##:set default format"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	SampleFormat format;
	int compression;
};

}
