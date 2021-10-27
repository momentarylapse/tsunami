/*
 * ActionTrackDetuneSynthesizer.h
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
#include "../../../Module/Synth/Synthesizer.h"
class Track;

class ActionTrackDetuneSynthesizer: public Action {
public:
	ActionTrackDetuneSynthesizer(Track *t, const float tuning[MAX_PITCH]);

	string name() const override { return ":##:detune synthesizer"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	float tuning[MAX_PITCH];
};
