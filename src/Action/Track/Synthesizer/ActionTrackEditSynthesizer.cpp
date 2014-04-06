/*
 * ActionTrackEditSynthesizer.cpp
 *
 *  Created on: 29.12.2013
 *      Author: michi
 */

#include "ActionTrackEditSynthesizer.h"
#include "../../../Data/Track.h"
#include "../../../Audio/Synth/Synthesizer.h"
#include <assert.h>

ActionTrackEditSynthesizer::ActionTrackEditSynthesizer(Track *t, const string &params_old)
{
	track_no = get_track_index(t);
	params = params_old;
	first_execution = true;
}

ActionTrackEditSynthesizer::~ActionTrackEditSynthesizer()
{
}

void *ActionTrackEditSynthesizer::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	// old_params... so don't execute on first run
	if (first_execution){
		first_execution = false;
		return NULL;
	}

	assert(track_no >= 0);
	assert(track_no < a->track.num);

	Track *t = a->get_track(track_no);
	assert(t);
	assert(t->synth);

	string temp = params;
	params = t->synth->ConfigToString();
	t->synth->ConfigFromString(temp);
	t->Notify();

	return NULL;
}

void ActionTrackEditSynthesizer::undo(Data *d)
{
	execute(d);
}

