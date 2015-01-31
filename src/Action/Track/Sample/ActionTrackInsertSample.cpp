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
#include "../../../Data/AudioFile.h"
#include "../Midi/ActionTrackAddMidiEvent.h"

ActionTrackInsertSample::ActionTrackInsertSample(AudioFile *a, int track_no, int index, int level_no)
{
	Track *t = a->track[track_no];
	SampleRef *sub = t->sample[index];
	if (t->type == t->TYPE_AUDIO){

		// get target buffer
		Range r = sub->getRange();
		addSubAction(new ActionTrackCreateBuffers(t, level_no, r), a);
		BufferBox buf = t->readBuffers(level_no, r);

		// insert sub (ignore muted)
		ActionTrackEditBuffer *action = new ActionTrackEditBuffer(t, level_no, r);
		buf.set(*sub->buf, 0, sub->volume);
		addSubAction(action, a);
	}else if (t->type == t->TYPE_MIDI){
		foreach(MidiEvent &e, *sub->midi){
			MidiEvent ee = e;
			ee.pos += sub->pos;
			addSubAction(new ActionTrackAddMidiEvent(t, ee), a);
		}
	}

	// delete sub
	addSubAction(new ActionTrackDeleteSample(t, index), a);
}

