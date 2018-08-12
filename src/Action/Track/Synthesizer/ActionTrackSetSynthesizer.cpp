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
	msg_write("notify 1");
	track->synth->notify(Synthesizer::MESSAGE_DELETE);
	Synthesizer *temp = synth;
	synth = track->synth;
	track->synth = temp;
	msg_write("notify 2");
	track->notify();

	return synth;
}

