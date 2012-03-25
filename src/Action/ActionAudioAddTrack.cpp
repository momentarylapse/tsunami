/*
 * ActionAudioAddTrack.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionAudioAddTrack.h"
#include "../Data/AudioFile.h"
#include "../lib/hui/hui.h"
#include <assert.h>

ActionAudioAddTrack::ActionAudioAddTrack(int _index)
{
	index = _index;
}

ActionAudioAddTrack::~ActionAudioAddTrack()
{
}

void ActionAudioAddTrack::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->track.erase(index);
	a->cur_track = old_cur_track;
}



void ActionAudioAddTrack::redo(Data *d)
{
	execute(d);
}



void *ActionAudioAddTrack::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	old_cur_track = a->cur_track;

	assert((index >= 0) && (index <= a->track.num));

	Track _dummy_, *t;
	a->cur_track = index;
	a->track.insert(_dummy_, a->cur_track);
	t = &a->track[a->cur_track];

	t->name = format(_("Spur %d"), a->track.num);
	t->root = a;
	t->parent = -1;
	t->is_selected = true;

	return t;
}


