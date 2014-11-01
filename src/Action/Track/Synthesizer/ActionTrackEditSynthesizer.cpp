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
	old_value = params_old;
	new_value = t->synth->configToString();
}

ActionTrackEditSynthesizer::~ActionTrackEditSynthesizer()
{
}

void *ActionTrackEditSynthesizer::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Track *t = a->get_track(track_no);
	assert(t);
	assert(t->synth);

	t->synth->configFromString(new_value);
	((Observable*)t->synth)->notify(t->synth->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionTrackEditSynthesizer::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Track *t = a->get_track(track_no);
	assert(t);
	assert(t->synth);

	t->synth->configFromString(old_value);
	((Observable*)t->synth)->notify(t->synth->MESSAGE_CHANGE_BY_ACTION);
}

bool ActionTrackEditSynthesizer::mergable(Action *a)
{
	ActionTrackEditSynthesizer *aa = dynamic_cast<ActionTrackEditSynthesizer*>(a);
	if (!aa)
		return false;
	return (aa->track_no == track_no);
}

