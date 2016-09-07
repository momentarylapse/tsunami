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
#include "../../../Data/Song.h"
#include "../Midi/ActionTrackAddMidiNote.h"

ActionTrackInsertSample::ActionTrackInsertSample(Song *s, int track_no, int index, int level_no)
{
	Track *t = s->tracks[track_no];
	SampleRef *sub = t->samples[index];
	if (t->type == t->TYPE_AUDIO){

		// get target buffer
		Range r = sub->range();
		addSubAction(new ActionTrackCreateBuffers(t, level_no, r), s);
		BufferBox buf = t->readBuffers(level_no, r);

		// insert sub (ignore muted)
		ActionTrackEditBuffer *action = new ActionTrackEditBuffer(t, level_no, r);
		buf.set(*sub->buf, 0, sub->volume);
		addSubAction(action, s);
	}else if (t->type == t->TYPE_MIDI){
		for (MidiNote *n : *sub->midi){
			MidiNote nn = *n;
			nn.range.offset += sub->pos;
			addSubAction(new ActionTrackAddMidiNote(t, nn), s);
		}
	}

	// delete sub
	addSubAction(new ActionTrackDeleteSample(t, index), s);
}

