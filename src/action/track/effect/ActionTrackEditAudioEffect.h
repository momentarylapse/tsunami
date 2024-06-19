/*
 * ActionTrackEditAudioEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#pragma once

#include "../../ActionMergable.h"

namespace tsunami {

class AudioEffect;

class ActionTrackEditAudioEffect: public ActionMergable<string> {
public:
	ActionTrackEditAudioEffect(AudioEffect *fx);

	string name() const override { return ":##:edit fx"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
	void redo(Data *d) override;

	bool mergable(Action *a) override;

private:
	AudioEffect *fx;
};

}
