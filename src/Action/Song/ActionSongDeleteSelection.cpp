/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "../Track/Buffer/ActionTrack__SplitBufferBox.h"
#include "../Track/Buffer/ActionTrack__DeleteBufferBox.h"
#include "../Track/Buffer/ActionTrack__ShrinkBufferBox.h"
#include "../Track/Sample/ActionTrackDeleteSample.h"
#include "ActionSongDeleteSelection.h"
#include "../Track/Midi/ActionTrackDeleteMidiNote.h"
#include "../../Data/SongSelection.h"

ActionSongDeleteSelection::ActionSongDeleteSelection(Song *a, int level_no, const SongSelection &sel, bool all_levels)
{
	Set<Track*> tracks = sel.tracks;
	for (Track *t : tracks){
		// buffer boxes
		if (all_levels){
			foreachi(TrackLevel &l, t->levels, li)
				DeleteBuffersFromTrackLevel(a, t, l, sel, li);
		}else{
			DeleteBuffersFromTrackLevel(a, t, t->levels[level_no], sel, level_no);
		}



		// subs
		Set<int> to_delete;
		foreachib(SampleRef *s, t->samples, i)
			if (sel.has(s))
				to_delete.add(i);

		for (int i : to_delete)
			addSubAction(new ActionTrackDeleteSample(t, i), a);

		// midi
		to_delete.clear();
		foreachib(MidiNote &n, t->midi, i){
			if (sel.range.is_inside(n.range.center()))
				to_delete.add(i);
		}

		for (int i : to_delete)
			addSubAction(new ActionTrackDeleteMidiNote(t, i), a);
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

		}else if ((i0 > bi0) and (i1 > bi1) and (i0 < bi1)){
			// overlapping end of b?
			addSubAction(new ActionTrack__ShrinkBufferBox(t, level_no, n, i0 - bi0), a);

		}else if ((i0 <= bi0) and (i1 < bi1) and (i1 > bi0)){
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

