/*
 * ActionBarAdd.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarAdd.h"

#include "../../Data/Track.h"
#include <assert.h>

#include "../../Data/Rhythm/Bar.h"
#include "../Track/Buffer/ActionTrack__SplitBuffer.h"
#include "../Track/Marker/ActionTrackEditMarker.h"
#include "../Track/Midi/ActionTrackEditMidiNote.h"
#include "Action__ShiftData.h"
#include "ActionBar__Add.h"

ActionBarAdd::ActionBarAdd(int _index, int length, int num_beats, int num_sub_beats, int _mode)
{
	index = _index;
	bar = new Bar(length, num_beats, num_sub_beats);
	mode = _mode;
}

ActionBarAdd::~ActionBarAdd()
{
	delete bar;
}

void ActionBarAdd::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	addSubAction(new ActionBar__Add(index, bar), d);

	if (mode != Bar::EditMode::IGNORE){
		int pos0 = s->barOffset(index);

		for (Track *t: s->tracks)
			for (int l=0; l<t->layers.num; l++)
				for (int i=t->layers[l].buffers.num-1; i>=0; i--)
					if (t->layers[l].buffers[i].range().is_more_inside(pos0))
						addSubAction(new ActionTrack__SplitBuffer(t, l, i, pos0 - t->layers[l].buffers[i].offset), d);

		addSubAction(new Action__ShiftData(pos0, bar->length, mode), d);

		Range r = Range(pos0, bar->length);

		for (Track *t: s->tracks){
			foreachi (TrackMarker *m, t->markers, i){
				if (m->range.is_inside(pos0)){
					// stretch
					addSubAction(new ActionTrackEditMarker(t, i, Range(m->range.offset, m->range.length + r.length), m->text), d);
				}
			}

			foreachi (MidiNote *m, t->midi, i){
				if (m->range.is_inside(pos0)){
					// stretch
					addSubAction(new ActionTrackEditMidiNote(t, m, Range(m->range.offset, m->range.length + r.length), m->pitch, m->volume), d);
				}
			}
		}
	}
}

