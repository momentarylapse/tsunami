/*
 * ActionAudioChangeAllTrackVolumes.cpp
 *
 *  Created on: 24.05.2015
 *      Author: michi
 */

#include "ActionAudioChangeAllTrackVolumes.h"
#include "../../../Data/AudioFile.h"

ActionAudioChangeAllTrackVolumes::ActionAudioChangeAllTrackVolumes(AudioFile *a, Track *t, float _volume)
{
	track_no = t->get_index();
	new_value = _volume;
	old_value = t->volume;
	foreach(Track *tt, t->root->tracks)
		old_volumes.add(tt->volume);
}

void *ActionAudioChangeAllTrackVolumes::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->volume = new_value;

	float factor = new_value / old_value;

	foreachi(Track *tt, a->tracks, i){
		tt->volume = old_volumes[i] * factor;
		tt->notify(tt->MESSAGE_CHANGE);
	}

	return NULL;
}

void ActionAudioChangeAllTrackVolumes::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	foreachi(Track *tt, a->tracks, i){
		tt->volume = old_volumes[i];
		tt->notify(tt->MESSAGE_CHANGE);
	}
}


bool ActionAudioChangeAllTrackVolumes::mergable(Action *a)
{
	ActionAudioChangeAllTrackVolumes *aa = dynamic_cast<ActionAudioChangeAllTrackVolumes*>(a);
	if (aa->track_no != track_no)
		return false;
	return aa;
}


