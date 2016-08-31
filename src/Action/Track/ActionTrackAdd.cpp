/*
 * ActionTrackAdd.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrackAdd.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../lib/hui/hui.h"
#include <assert.h>
#include "../../Data/Song.h"

ActionTrackAdd::ActionTrackAdd(int _index, int _type)
{
	index = _index;
	type = _type;
}

ActionTrackAdd::~ActionTrackAdd()
{
}

void ActionTrackAdd::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	delete(a->tracks[index]);
	a->tracks.erase(index);
	a->notify(a->MESSAGE_DELETE_TRACK);
}



void *ActionTrackAdd::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	assert((index >= 0) and (index <= a->tracks.num));

	Track *t = new Track;

	t->song = a;
	t->type = type;
	t->levels.resize(a->level_names.num);

	a->tracks.insert(t, index);

	a->notify(a->MESSAGE_ADD_TRACK);

	return t;
}


