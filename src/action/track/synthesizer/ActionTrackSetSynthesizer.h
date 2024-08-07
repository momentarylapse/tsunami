/*
 * ActionTrackSetSynthesizer.h
 *
 *  Created on: 28.12.2013
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class Track;
class Synthesizer;

class ActionTrackSetSynthesizer : public Action {
public:
	ActionTrackSetSynthesizer(Track *t, shared<Synthesizer> synth);

	string name() const override { return ":##:set synthesizer"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	shared<Synthesizer> synth;
};

}
