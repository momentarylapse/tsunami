/*
 * ActionTrackDetuneSynthesizer.cpp
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#include "ActionTrackDetuneSynthesizer.h"
#include <assert.h>
#include "../../../Data/Song.h"
#include "../../../Module/Synth/Synthesizer.h"

ActionTrackDetuneSynthesizer::ActionTrackDetuneSynthesizer(Track *t, int pitch, float dpitch, bool all_octaves)
{
	track_no = t->get_index();
	tuning = t->synth->tuning;

	if (all_octaves){
		for (int i=(pitch % 12); i<MAX_PITCH; i+=12)
			tuning.freq[i] *= pitch_to_freq(dpitch) / pitch_to_freq(0);
	}else
		tuning.freq[pitch] *= pitch_to_freq(dpitch) / pitch_to_freq(0);
}

void *ActionTrackDetuneSynthesizer::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	assert((track_no >= 0) and (track_no <= a->tracks.num));

	Track *t = a->tracks[track_no];

	Synthesizer::Tuning temp = tuning;
	tuning = t->synth->tuning;
	t->synth->tuning = temp;
	t->synth->update_delta_phi();

	t->synth->Observable::notify(t->synth->MESSAGE_CHANGE_BY_ACTION);

	return t;
}

void ActionTrackDetuneSynthesizer::undo(Data *d)
{
	execute(d);
}
