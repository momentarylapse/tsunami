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
#include "../../../Data/Track.h"
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
	Song *s = dynamic_cast<Song*>(d);

	Track *t = layer->track;
	SampleRef *ref = layer->samples[index];
	Sample *sample = ref->origin;
	if (t->type == SignalType::AUDIO){

		// get target buffer
		Range r = ref->range();
		addSubAction(new ActionTrackCreateBuffers(layer, r), s);
		AudioBuffer buf;
		layer->readBuffers(buf, r, true);

		// insert sub (ignore muted)
		ActionTrackEditBuffer *action = new ActionTrackEditBuffer(layer, r);
		buf.set(*ref->buf, 0, ref->volume);
		addSubAction(action, s);
	}else if (t->type == SignalType::MIDI){
		for (MidiNote *n : *ref->midi){
			MidiNote *nn = n->copy();
			nn->range.offset += ref->pos;
			addSubAction(new ActionTrackAddMidiNote(layer, nn), s);
		}
	}

	// delete sub
	addSubAction(new ActionTrackDeleteSample(ref), s);

	if (sample->auto_delete and (sample->ref_count == 0))
		addSubAction(new ActionSampleDelete(sample), d);
}

