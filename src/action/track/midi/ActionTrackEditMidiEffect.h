/*
 * ActionTrackEditMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>

namespace tsunami {

struct Track;
class MidiEffect;

class ActionTrackEditMidiEffect: public history::MergableValueAction<string> {
public:
	ActionTrackEditMidiEffect(MidiEffect *fx);

	string name() const override { return ":##:edit midi fx"; }

	void* execute(history::Data* d) override;
	void redo(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;

private:
	MidiEffect *fx;
};

}
