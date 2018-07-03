/*
 * ActionTrackAdd.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrackAdd.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Data/Song.h"
#include "../../lib/hui/hui.h"
#include <assert.h>

ActionTrackAdd::ActionTrackAdd(int _type, int _index)
{
	index = _index;
	type = _type;
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

	Track *t = new Track(type, CreateSynthesizer(a->session, "Dummy"));

	t->song = a;

	a->tracks.insert(t, index);

	a->notify(a->MESSAGE_ADD_TRACK);

	return t;
}


