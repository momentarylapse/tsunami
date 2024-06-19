/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionSongDeleteSelection.h"
#include "../track/sample/ActionTrackDeleteSample.h"
#include "../track/marker/ActionTrackDeleteMarker.h"
#include "../track/midi/ActionTrackDeleteMidiNote.h"
#include "../../data/SongSelection.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/SampleRef.h"
#include "../track/buffer/ActionTrack__DeleteBuffer.h"
#include "../track/buffer/ActionTrack__ShrinkBuffer.h"
#include "../track/buffer/ActionTrack__SplitBuffer.h"

namespace tsunami {

ActionSongDeleteSelection::ActionSongDeleteSelection(const SongSelection &_sel) :
	sel(_sel)
{
}

void ActionSongDeleteSelection::build(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	for (auto t: weak(s->tracks)) {

		for (auto l: weak(t->layers)) {
			// buffer boxes
			if (sel.has(l) and !sel.range().is_none())
				delete_buffers_from_track_layer(s, t, l, sel);

			// midi
			for (int i=l->midi.num-1; i>=0; i--)
				if (sel.has(l->midi[i].get()))
					add_sub_action(new ActionTrackDeleteMidiNote(l, i), d);

			// marker
			for (int i=l->markers.num-1; i>=0; i--)
				if (sel.has(l->markers[i].get()))
					add_sub_action(new ActionTrackDeleteMarker(l, i), d);

			// samples
			for (int i=l->samples.num-1; i>=0; i--)
				if (sel.has(l->samples[i].get()))
					add_sub_action(new ActionTrackDeleteSample(l->samples[i]), d);
		}
	}
}

void ActionSongDeleteSelection::delete_buffers_from_track_layer(Song* a, Track *t, TrackLayer *l, const SongSelection &sel) {
	int i0 = sel.range().start();
	int i1 = sel.range().end();
	foreachib(AudioBuffer &b, l->buffers, n) {
		int bi0 = b.offset;
		int bi1 = b.offset + b.length;


		if (sel.range().covers(b.range())) {
			// b completely inside?
			add_sub_action(new ActionTrack__DeleteBuffer(l, n), a);

		} else if (sel.range().is_inside(bi1-1)) {
			// overlapping end of b?
			add_sub_action(new ActionTrack__ShrinkBuffer(l, n, i0 - bi0), a);

		} else if (sel.range().is_inside(bi0)) {
			// overlapping beginning of b?
			add_sub_action(new ActionTrack__SplitBuffer(l, n, i1 - bi0), a);
			add_sub_action(new ActionTrack__DeleteBuffer(l, n), a);

		} else if (b.range().covers(sel.range())) {
			// inside b?
			add_sub_action(new ActionTrack__SplitBuffer(l, n, i1 - bi0), a);
			add_sub_action(new ActionTrack__SplitBuffer(l, n, i0 - bi0), a);
			add_sub_action(new ActionTrack__DeleteBuffer(l, n + 1), a);
		}
		_foreach_it_.update();
	}
}

}

