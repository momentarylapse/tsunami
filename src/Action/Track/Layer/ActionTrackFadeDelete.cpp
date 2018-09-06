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
	index = _index;
}

void* ActionTrackFadeDelete::execute(Data* d)
{
	fade = track->fades[index];
	track->fades.erase(index);

	return nullptr;
}

void ActionTrackFadeDelete::undo(Data* d)
{
	track->fades.insert(fade, index);

}
