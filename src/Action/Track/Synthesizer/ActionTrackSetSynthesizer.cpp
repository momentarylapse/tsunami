/*
 * ActionTrackSetSynthesizer.cpp
 *
 *  Created on: 28.12.2013
 *      Author: michi
 */

#include "ActionTrackSetSynthesizer.h"
#include "../../../Data/AudioFile.h"
#include <assert.h>

ActionTrackSetSynthesizer::ActionTrackSetSynthesizer(Track *t, Synthesizer *_synth)
{
	track_no = get_track_index(t);
	synth = _synth;
}

ActionTrackSetSynthesizer::~ActionTrackSetSynthesizer()
{
}

void ActionTrackSetSynthesizer::undo(Data *d)
{
	execute(d);
}


void *ActionTrackSetSynthesizer::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	assert((track_no >= 0) && (track_no <= a->tracks.num));

	Track *t = a->tracks[track_no];

	Synthesizer *temp = synth;
	synth = t->synth;
	t->synth = temp;
	t->notify();

	return t;
}

