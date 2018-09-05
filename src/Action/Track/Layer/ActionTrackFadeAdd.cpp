/*
 * ActionTrackFadeAdd.cpp
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#include "ActionTrackFadeAdd.h"
#include "../../../Data/Track.h"


ActionTrackFadeAdd::ActionTrackFadeAdd(Track* t, int _position, int _samples, int _target)
{
	track = t;
	position = _position;
	samples = _samples;
	target = _target;
	index = 0;
}

void* ActionTrackFadeAdd::execute(Data* d)
{
	Track::Fade f;
	f.position = position;
	f.samples = samples;
	f.target = target;
	for (int i=0; i<track->fades.num; i++)
		if (track->fades[i].position < f.position){
			index = i + 1;
		}
	track->fades.insert(f, index);

	return nullptr;
}

void ActionTrackFadeAdd::undo(Data* d)
{
	track->fades.erase(index);

}
