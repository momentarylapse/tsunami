/*
 * ActionBarAdd.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarAdd.h"

#include <assert.h>

#include "../../data/rhythm/Bar.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/Song.h"
#include "../../data/audio/AudioBuffer.h"
#include "../track/buffer/ActionTrack__SplitBuffer.h"
#include "../track/marker/ActionTrackEditMarker.h"
#include "../track/midi/ActionTrackEditMidiNote.h"
#include "Action__ShiftData.h"
#include "ActionBar__Add.h"

namespace tsunami {

ActionBarAdd::ActionBarAdd(int _index, const BarPattern &pattern, BarEditMode _mode) {
	index = _index;
	bar = new Bar(pattern);
	mode = _mode;
}

void ActionBarAdd::build(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	add_sub_action(new ActionBar__Add(index, bar), d);

	if (mode != BarEditMode::Ignore) {
		int pos0 = s->bar_offset(index);

		for (auto t: s->tracks)
			for (auto l: t->layers)
				for (int i=l->buffers.num-1; i>=0; i--)
					if (l->buffers[i].range().is_more_inside(pos0))
						add_sub_action(new ActionTrack__SplitBuffer(l, i, pos0 - l->buffers[i].offset), d);

		add_sub_action(new Action__ShiftData(pos0, bar->length, mode), d);

		Range r = Range(pos0, bar->length);

		for (auto t: weak(s->tracks)) {
			for (auto l: weak(t->layers)) {
				foreachi (auto m, weak(l->markers), i) {
					if (m->range.is_inside(pos0)) {
						// stretch
						add_sub_action(new ActionTrackEditMarker(l, m, Range(m->range.offset, m->range.length + r.length), m->text), d);
					}
				}

				foreachi (auto m, l->midi, i) {
					if (m->range.is_inside(pos0)) {
						// stretch
						add_sub_action(new ActionTrackEditMidiNote(l, m, Range(m->range.offset, m->range.length + r.length), m->pitch, m->volume, m->stringno, m->flags), d);
					}
				}
			}
		}
	}
}

}

