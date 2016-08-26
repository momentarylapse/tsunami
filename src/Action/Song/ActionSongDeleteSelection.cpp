/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionSongDeleteSelection.h"
#include "../Track/Buffer/ActionTrack__SplitBufferBox.h"
#include "../Track/Buffer/ActionTrack__DeleteBufferBox.h"
#include "../Track/Buffer/ActionTrack__ShrinkBufferBox.h"
#include "../Track/Sample/ActionTrackDeleteSample.h"
#include "../Track/Marker/ActionTrackDeleteMarker.h"
#include "../Track/Midi/ActionTrackDeleteMidiNote.h"
#include "../../Data/SongSelection.h"

ActionSongDeleteSelection::ActionSongDeleteSelection(Song *s, int level_no, const SongSelection &sel, bool all_levels)
{
	for (Track *t : s->tracks){
		if (!sel.has(t))
			continue;

		// buffer boxes
		if (all_levels){
			foreachi(TrackLevel &l, t->levels, li)
				DeleteBuffersFromTrackLevel(s, t, l, sel, li);
		}else{
			DeleteBuffersFromTrackLevel(s, t, t->levels[level_no], sel, level_no);
		}



		// subs
		for (int i=t->samples.num-1; i>=0; i--)
			if (sel.has(t->samples[i]))
				addSubAction(new ActionTrackDeleteSample(t, i), s);

		// midi
		for (int i=t->midi.num-1; i>=0; i--)
			if (sel.has(&t->midi[i]))
				addSubAction(new ActionTrackDeleteMidiNote(t, i), s);

		// marker
		for (int i=t->markers.num-1; i>=0; i--)
			if (sel.has(&t->markers[i]))
				addSubAction(new ActionTrackDeleteMarker(t, i), s);
	}
}

void ActionSongDeleteSelection::DeleteBuffersFromTrackLevel(Song* a, Track *t, TrackLevel& l, const SongSelection &sel, int level_no)
{
	int i0 = sel.range.start();
	int i1 = sel.range.end();
	foreachib(BufferBox &b, l.buffers, n){
		int bi0 = b.offset;
		int bi1 = b.offset + b.length;


		if (sel.range.covers(b.range())){
			// b completely inside?
			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n), a);

		}else if (sel.range.is_inside(bi1-1)){//((i0 > bi0) and (i1 > bi1) and (i0 < bi1)){
			// overlapping end of b?
			addSubAction(new ActionTrack__ShrinkBufferBox(t, level_no, n, i0 - bi0), a);

		}else if (sel.range.is_inside(bi0)){//((i0 <= bi0) and (i1 < bi1) and (i1 > bi0)){
			// overlapping beginning of b?
			addSubAction(new ActionTrack__SplitBufferBox(t, level_no, n, i1 - bi0), a);
			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n), a);

		}else if (b.range().covers(sel.range)){
			// inside b?
			addSubAction(new ActionTrack__SplitBufferBox(t, level_no, n, i1 - bi0), a);
			addSubAction(new ActionTrack__SplitBufferBox(t, level_no, n, i0 - bi0), a);
			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n + 1), a);

		}
		_foreach_it_.update();
	}
}

