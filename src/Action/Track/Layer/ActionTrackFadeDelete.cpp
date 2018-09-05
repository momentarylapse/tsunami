/*
 * ActionTrackFadeDelete.cpp
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#include "ActionTrackFadeDelete.h"
#include "../../../Data/Track.h"


ActionTrackFadeDelete::ActionTrackFadeDelete(Track* t, int _index)
{
	track = t;
	position = samples = target = 0;
	index = _index;
}

void* ActionTrackFadeDelete::execute(Data* d)
{
	position = track->fades[index].position;
	samples = track->fades[index].samples;
	target = track->fades[index].target;
	track->fades.erase(index);

	return nullptr;
}

void ActionTrackFadeDelete::undo(Data* d)
{
	Track::Fade f;
	f.position = position;
	f.samples = samples;
	f.target = target;
	track->fades.insert(f, index);

}
