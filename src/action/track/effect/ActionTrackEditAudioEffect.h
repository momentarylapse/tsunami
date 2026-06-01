/*
 * ActionTrackEditAudioEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>

namespace tsunami {

class AudioEffect;

class ActionTrackEditAudioEffect: public history::MergableValueAction<string> {
public:
	ActionTrackEditAudioEffect(AudioEffect *fx);

	string name() const override { return ":##:edit fx"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
	void redo(history::Data* d) override;

	bool mergable(Action *a) override;

private:
	AudioEffect *fx;
};

}
