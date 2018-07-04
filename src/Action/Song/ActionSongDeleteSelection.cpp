/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionSongDeleteSelection.h"
#include "../Track/Sample/ActionTrackDeleteSample.h"
#include "../Track/Marker/ActionTrackDeleteMarker.h"
#include "../Track/Midi/ActionTrackDeleteMidiNote.h"
#include "../../Data/SongSelection.h"
#include "../Track/Buffer/ActionTrack__DeleteBuffer.h"
#include "../Track/Buffer/ActionTrack__ShrinkBuffer.h"
#include "../Track/Buffer/ActionTrack__SplitBuffer.h"

ActionSongDeleteSelection::ActionSongDeleteSelection(const SongSelection &_sel) :
	sel(_sel)
{
}

void ActionSongDeleteSelection::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	for (Track *t: s->tracks){

		// samples
		for (int i=t->samples.num-1; i>=0; i--)
			if (sel.has(t->samples[i]))
				addSubAction(new ActionTrackDeleteSample(t->samples[i]), d);

		// midi
		for (int i=t->midi.num-1; i>=0; i--)
			if (sel.has(t->midi[i]))
				addSubAction(new ActionTrackDeleteMidiNote(t, i), d);

		// marker
		for (int i=t->markers.num-1; i>=0; i--)
			if (sel.has(t->markers[i]))
				addSubAction(new ActionTrackDeleteMarker(t, i), d);


		if (!sel.has(t) or sel.range.empty())
			continue;

		// buffer boxes
		for (TrackLayer *l: t->layers)
			if (sel.has(l))
				DeleteBuffersFromTrackLayer(s, t, l, sel);
	}
}

void ActionSongDeleteSelection::DeleteBuffersFromTrackLayer(Song* a, Track *t, TrackLayer *l, const SongSelection &sel)
{
	int i0 = sel.range.start();
	int i1 = sel.range.end();
	foreachib(AudioBuffer &b, l->buffers, n){
		int bi0 = b.offset;
		int bi1 = b.offset + b.length;


		if (sel.range.covers(b.range())){
			// b completely inside?
			addSubAction(new ActionTrack__DeleteBuffer(l, n), a);

		}else if (sel.range.is_inside(bi1-1)){
			// overlapping end of b?
			addSubAction(new ActionTrack__ShrinkBuffer(l, n, i0 - bi0), a);

		}else if (sel.range.is_inside(bi0)){
			// overlapping beginning of b?
			addSubAction(new ActionTrack__SplitBuffer(l, n, i1 - bi0), a);
			addSubAction(new ActionTrack__DeleteBuffer(l, n), a);

		}else if (b.range().covers(sel.range)){
			// inside b?
			addSubAction(new ActionTrack__SplitBuffer(l, n, i1 - bi0), a);
			addSubAction(new ActionTrack__SplitBuffer(l, n, i0 - bi0), a);
			addSubAction(new ActionTrack__DeleteBuffer(l, n + 1), a);

		}
		_foreach_it_.update();
	}
}

