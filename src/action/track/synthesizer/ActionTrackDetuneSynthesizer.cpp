/*
 * ActionTrackDetuneSynthesizer.cpp
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#include "ActionTrackDetuneSynthesizer.h"
#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../module/synthesizer/Synthesizer.h"
#include <assert.h>

namespace tsunami {

ActionTrackDetuneSynthesizer::ActionTrackDetuneSynthesizer(Track *t, const Temperament &_temp) {
	track = t;
	temperament = _temp;
}

void *ActionTrackDetuneSynthesizer::execute(Data *d) {
	std::swap(track->synth->temperament, temperament);
	track->synth->update_delta_phi();

	track->synth->out_changed.notify();

	return nullptr;
}

void ActionTrackDetuneSynthesizer::undo(Data *d) {
	execute(d);
}

}
