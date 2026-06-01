/*
 * ActionTrackSetSynthesizer.h
 *
 *  Created on: 28.12.2013
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class Track;
class Synthesizer;

class ActionTrackSetSynthesizer : public history::Action {
public:
	ActionTrackSetSynthesizer(Track *t, shared<Synthesizer> synth);

	string name() const override { return ":##:set synthesizer"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	Track *track;
	shared<Synthesizer> synth;
};

}
