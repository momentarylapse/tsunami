/*
 * ActionSongAddSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "../../Song/Sample/ActionSongAddSample.h"

#include <assert.h>
#include "../../../Data/Song.h"

ActionSongAddSample::ActionSongAddSample(const string &name, BufferBox &buf)
{
	sample = new Sample(Track::TYPE_AUDIO);
	sample->buf = buf;
	sample->buf.offset = 0;
	sample->name = name;
}

ActionSongAddSample::ActionSongAddSample(const string &name, MidiNoteData &midi)
{
	sample = new Sample(Track::TYPE_MIDI);
	sample->midi = midi;
	sample->midi.sort();
	sample->name = name;
}

ActionSongAddSample::~ActionSongAddSample()
{
	if (!sample->owner)
		delete(sample);
}

void *ActionSongAddSample::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	sample->owner = a;
	a->samples.add(sample);
	a->notify(a->MESSAGE_ADD_SAMPLE);
	return sample;
}

void ActionSongAddSample::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(sample->ref_count == 0);
	sample->notify(sample->MESSAGE_DELETE);
	a->samples.pop();
	sample->owner = NULL;
	a->notify(a->MESSAGE_DELETE_SAMPLE);
}

