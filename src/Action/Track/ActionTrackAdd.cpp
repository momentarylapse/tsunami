/*
 * ActionTrackAdd.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrackAdd.h"
#include "../../Data/AudioFile.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../lib/hui/hui.h"
#include <assert.h>

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
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->track[index]->Notify("Delete");
	a->Notify("DeleteTrack");
	delete(a->track[index]);
	a->track.erase(index);
}



void *ActionTrackAdd::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	assert((index >= 0) && (index <= a->track.num));

	Track *t = new Track;

	t->name = format(_("Spur %d"), a->track.num + 1);
	t->root = a;
	t->is_selected = true;
	t->type = type;
	t->level.resize(a->level_name.num);

	a->track.insert(t, index);

	a->Notify("AddTrack");

	return t;
}


