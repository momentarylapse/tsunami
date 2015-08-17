/*
 * ActionSongChangeAllTrackVolumes.cpp
 *
 *  Created on: 24.05.2015
 *      Author: michi
 */

#include "ActionSongChangeAllTrackVolumes.h"
#include "../../../Data/Song.h"

ActionSongChangeAllTrackVolumes::ActionSongChangeAllTrackVolumes(Song *s, Track *t, float _volume)
{
	track_no = t->get_index();
	new_value = _volume;
	old_value = t->volume;
	foreach(Track *tt, t->song->tracks)
		old_volumes.add(tt->volume);
}

void *ActionSongChangeAllTrackVolumes::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	s->volume = new_value;

	float factor = new_value / old_value;

	foreachi(Track *tt, s->tracks, i){
		tt->volume = old_volumes[i] * factor;
		tt->notify(tt->MESSAGE_CHANGE);
	}

	return NULL;
}

void ActionSongChangeAllTrackVolumes::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	foreachi(Track *tt, s->tracks, i){
		tt->volume = old_volumes[i];
		tt->notify(tt->MESSAGE_CHANGE);
	}
}


bool ActionSongChangeAllTrackVolumes::mergable(Action *a)
{
	ActionSongChangeAllTrackVolumes *aa = dynamic_cast<ActionSongChangeAllTrackVolumes*>(a);
	if (aa->track_no != track_no)
		return false;
	return aa;
}


