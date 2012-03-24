/*
 * ActionAudioAddTrack.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionAudioAddTrack.h"
#include "../Data/AudioFile.h"
#include "../lib/hui/hui.h"

ActionAudioAddTrack::ActionAudioAddTrack(int _index)
{
	index = _index;
}

ActionAudioAddTrack::~ActionAudioAddTrack()
{
}

void ActionAudioAddTrack::undo(Data *d)
{
}



void ActionAudioAddTrack::redo(Data *d)
{
}



void *ActionAudioAddTrack::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Track _dummy_, *t;
	if (index < 0){
		a->track.add(_dummy_);
		t = &a->track.back();
		a->cur_track = a->track.num - 1;
	}else{
		a->cur_track = index + 1;
		a->track.insert(_dummy_, a->cur_track);
		t = &a->track[a->cur_track];
	}

	t->name = format(_("Spur %d"), a->track.num);
	t->root = a;
	t->parent = -1;
	t->is_selected = true;

	return t;
}


