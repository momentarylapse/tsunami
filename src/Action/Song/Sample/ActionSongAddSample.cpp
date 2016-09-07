/*
 * ActionSongAddSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "../../Song/Sample/ActionSongAddSample.h"

#include <assert.h>
#include "../../../Data/Song.h"

ActionSongAddSample::ActionSongAddSample(const string &name, const BufferBox &buf)
{
	sample = new Sample(Track::TYPE_AUDIO);
	sample->buf = buf;
	sample->buf.offset = 0;
	sample->name = name;
}

ActionSongAddSample::ActionSongAddSample(const string &name, const MidiData &midi)
{
	msg_write("sa");
	sample = new Sample(Track::TYPE_MIDI);
	msg_write("sb");
	sample->midi = midi;
	msg_write("sc");
	sample->midi.sort();
	sample->name = name;
	msg_write("sd");
}

ActionSongAddSample::~ActionSongAddSample()
{
	if (!sample->owner)
		delete(sample);
}

void *ActionSongAddSample::execute(Data *d)
{
	msg_write("sxa");
	Song *a = dynamic_cast<Song*>(d);
	sample->owner = a;
	a->samples.add(sample);
	a->notify(a->MESSAGE_ADD_SAMPLE);
	msg_write("sxz");
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

