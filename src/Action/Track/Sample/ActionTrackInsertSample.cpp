/*
 * ActionTrackInsertSample.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackInsertSample.h"
#include "ActionTrackDeleteSample.h"
#include "../Buffer/ActionTrackCreateBuffers.h"
#include "../Buffer/ActionTrackEditBuffer.h"
#include "../Midi/ActionTrackAddMidiNote.h"
#include "../../../Data/AudioFile.h"

ActionTrackInsertSample::ActionTrackInsertSample(AudioFile *a, int track_no, int index, int level_no)
{
	Track *t = a->track[track_no];
	SampleRef *sub = t->sample[index];
	if (t->type == t->TYPE_AUDIO){

		// get target buffer
		Range r = sub->GetRange();
		AddSubAction(new ActionTrackCreateBuffers(t, level_no, r), a);
		BufferBox buf = t->ReadBuffers(level_no, r);

		// insert sub (ignore muted)
		ActionTrackEditBuffer *action = new ActionTrackEditBuffer(t, level_no, r);
		buf.set(*sub->buf, 0, sub->volume);
		AddSubAction(action, a);
	}else if (t->type == t->TYPE_MIDI){
		foreach(MidiNote &n, *sub->midi){
			MidiNote nn = n;
			nn.range.offset += sub->pos;
			AddSubAction(new ActionTrackAddMidiNote(t, nn), a);
		}
	}

	// delete sub
	AddSubAction(new ActionTrackDeleteSample(t, index), a);
}

ActionTrackInsertSample::~ActionTrackInsertSample()
{
}

