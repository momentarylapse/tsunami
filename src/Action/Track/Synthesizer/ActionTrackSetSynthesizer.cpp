/*
 * ActionTrackSetSynthesizer.cpp
 *
 *  Created on: 28.12.2013
 *      Author: michi
 */

#include "ActionTrackSetSynthesizer.h"
#include <assert.h>
#include "../../../Data/Track.h"
#include "../../../Module/Synth/Synthesizer.h"

ActionTrackSetSynthesizer::ActionTrackSetSynthesizer(Track *t, Synthesizer *_synth) {
	track = t;
	synth = _synth;
}

void ActionTrackSetSynthesizer::undo(Data *d) {
	execute(d);
}


void *ActionTrackSetSynthesizer::execute(Data *d) {
	track->synth->fake_death();
	std::swap(synth, track->synth);
	track->notify(Track::MESSAGE_REPLACE_SYNTHESIZER);

	return synth.get();
}

