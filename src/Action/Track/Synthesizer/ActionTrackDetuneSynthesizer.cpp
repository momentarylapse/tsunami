/*
 * ActionTrackDetuneSynthesizer.cpp
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#include "ActionTrackDetuneSynthesizer.h"
#include <assert.h>
#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Module/Synth/Synthesizer.h"

ActionTrackDetuneSynthesizer::ActionTrackDetuneSynthesizer(Track *t, const float _tuning[MAX_PITCH]) {
	track = t;
	memcpy(tuning, _tuning, sizeof(tuning));
}

void *ActionTrackDetuneSynthesizer::execute(Data *d) {
	for (int i=0; i<MAX_PITCH; i++)
		std::swap(track->synth->tuning.freq[i], tuning[i]);
	track->synth->update_delta_phi();

	track->synth->notify();

	return nullptr;
}

void ActionTrackDetuneSynthesizer::undo(Data *d) {
	execute(d);
}
