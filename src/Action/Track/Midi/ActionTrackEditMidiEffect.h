/*
 * ActionTrackEditMidiEffect.h
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#pragma once

#include "../../ActionMergable.h"
class Track;
class MidiEffect;

class ActionTrackEditMidiEffect: public ActionMergable<string> {
public:
	ActionTrackEditMidiEffect(MidiEffect *fx);

	string name() const override { return ":##:edit midi fx"; }

	void *execute(Data *d) override;
	void redo(Data *d) override;
	void undo(Data *d) override;

	bool mergable(Action *a) override;

private:
	MidiEffect *fx;
};
