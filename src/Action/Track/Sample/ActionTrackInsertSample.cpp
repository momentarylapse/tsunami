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
#include "../../Sample/ActionSampleDelete.h"

ActionTrackInsertSample::ActionTrackInsertSample(int _track_no, int _index, int _layer_no)
{
	track_no = _track_no;
	index = _index;
	layer_no = _layer_no;
}

void ActionTrackInsertSample::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	Track *t = s->tracks[track_no];
	SampleRef *ref = t->samples[index];
	Sample *sample = ref->origin;
	if (t->type == t->TYPE_AUDIO){

		// get target buffer
		Range r = ref->range();
		addSubAction(new ActionTrackCreateBuffers(t, layer_no, r), s);
		BufferBox buf = t->readBuffers(layer_no, r);

		// insert sub (ignore muted)
		ActionTrackEditBuffer *action = new ActionTrackEditBuffer(t, layer_no, r);
		buf.set(*ref->buf, 0, ref->volume);
		addSubAction(action, s);
	}else if (t->type == t->TYPE_MIDI){
		for (MidiNote *n : *ref->midi){
			MidiNote nn = *n;
			nn.range.offset += ref->pos;
			addSubAction(new ActionTrackAddMidiNote(t, nn), s);
		}
	}

	// delete sub
	addSubAction(new ActionTrackDeleteSample(ref), s);

	if (sample->auto_delete and (sample->ref_count == 0))
		addSubAction(new ActionSampleDelete(sample), d);
}

