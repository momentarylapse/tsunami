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

ActionTrackSetSynthesizer::ActionTrackSetSynthesizer(Track *t, Synthesizer *_synth)
{
	track = t;
	synth = _synth;
}

ActionTrackSetSynthesizer::~ActionTrackSetSynthesizer()
{
	delete synth;
}

void ActionTrackSetSynthesizer::undo(Data *d)
{
	execute(d);
}


void *ActionTrackSetSynthesizer::execute(Data *d)
{
	Synthesizer *temp = synth;
	synth = track->synth;
	track->synth = temp;
	track->notify();

	return synth;
}

