/*
 * ActionTrackSetSynthesizer.cpp
 *
 *  Created on: 28.12.2013
 *      Author: michi
 */

#include "ActionTrackSetSynthesizer.h"
#include "../../../data/Track.h"
#include "../../../module/synthesizer/Synthesizer.h"
#include <assert.h>

ActionTrackSetSynthesizer::ActionTrackSetSynthesizer(Track *t, shared<Synthesizer> _synth) {
	track = t;
	synth = _synth;
}

void ActionTrackSetSynthesizer::undo(Data *d) {
	execute(d);
}


void *ActionTrackSetSynthesizer::execute(Data *d) {
	track->synth->fake_death();
	std::swap(synth, track->synth);
	track->synth->set_instrument(track->instrument);
	track->out_replace_synthesizer.notify();

	return synth.get();
}

