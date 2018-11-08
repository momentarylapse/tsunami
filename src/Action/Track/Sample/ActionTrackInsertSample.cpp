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
#include "../../../Data/base.h"
#include "../../../Data/Song.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/Sample.h"
#include "../../../Data/SampleRef.h"
#include "../Midi/ActionTrackAddMidiNote.h"
#include "../../Sample/ActionSampleDelete.h"

ActionTrackInsertSample::ActionTrackInsertSample(TrackLayer *l, int _index)
{
	layer = l;
	index = _index;
}

void ActionTrackInsertSample::build(Data *d)
{
	SampleRef *ref = layer->samples[index];
	Sample *sample = ref->origin;
	if (layer->type == SignalType::AUDIO){

		// get target buffer
		Range r = ref->range();
		add_sub_action(new ActionTrackCreateBuffers(layer, r), d);
		AudioBuffer buf;
		layer->read_buffers(buf, r, true);

		// insert sub (ignore muted)
		ActionTrackEditBuffer *action = new ActionTrackEditBuffer(layer, r);
		buf.set(*ref->buf, 0, ref->volume);
		add_sub_action(action, d);
	}else if (layer->type == SignalType::MIDI){
		for (MidiNote *n : *ref->midi){
			MidiNote *nn = n->copy();
			nn->range.offset += ref->pos;
			add_sub_action(new ActionTrackAddMidiNote(layer, nn), d);
		}
	}

	// delete sub
	add_sub_action(new ActionTrackDeleteSample(ref), d);

	if (sample->auto_delete and (sample->ref_count == 0))
		add_sub_action(new ActionSampleDelete(sample), d);
}

