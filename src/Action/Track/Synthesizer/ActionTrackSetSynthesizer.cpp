/*
 * ActionTrackSetSynthesizer.cpp
 *
 *  Created on: 28.12.2013
 *      Author: michi
 */

#include "ActionTrackSetSynthesizer.h"
#include <assert.h>
#include "../../../Data/Song.h"

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
	Song *a = dynamic_cast<Song*>(d);

	assert((track_no >= 0) && (track_no <= a->tracks.num));

	Track *t = a->tracks[track_no];

	Synthesizer *temp = synth;
	synth = t->synth;
	t->synth = temp;
	t->notify();

	return t;
}

