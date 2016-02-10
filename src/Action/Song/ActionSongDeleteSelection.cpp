/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "../Track/Buffer/ActionTrack__CutBufferBox.h"
#include "../Track/Buffer/ActionTrack__DeleteBufferBox.h"
#include "../Track/Buffer/ActionTrack__ShrinkBufferBox.h"
#include "../Track/Sample/ActionTrackDeleteSample.h"
#include "ActionSongDeleteSelection.h"
#include "../Track/Midi/ActionTrackDeleteMidiNote.h"

ActionSongDeleteSelection::ActionSongDeleteSelection(Song *a, int level_no, const Range &range, const Array<Track*> &tracks, bool all_levels)
{
	foreach(Track *t, const_cast<Array<Track*>&>(tracks)){
		// buffer boxes
		if (all_levels){
			foreachi(TrackLevel &l, t->levels, li)
				DeleteBuffersFromTrackLevel(a, t, l, range, li);
		}else{
			DeleteBuffersFromTrackLevel(a, t, t->levels[level_no], range, level_no);
		}

		// subs
		foreachib(SampleRef *s, t->samples, i)
			if (s->is_selected){
				addSubAction(new ActionTrackDeleteSample(t, i), a);
				_foreach_it_.update(); // TODO...
			}

		// midi
		Set<int> to_delete;
		foreachi(MidiNote &n, t->midi, i){
			if (range.is_inside(n.range.center()))
				to_delete.add(i);
		}

		foreachb(int i, to_delete)
			addSubAction(new ActionTrackDeleteMidiNote(t, i), a);
	}
}

void ActionSongDeleteSelection::DeleteBuffersFromTrackLevel(Song* a, Track *t, TrackLevel& l, const Range &range, int level_no)
{
	int i0 = range.start();
	int i1 = range.end();
	foreachib(BufferBox &b, l.buffers, n){
		int bi0 = b.offset;
		int bi1 = b.offset + b.num;


		if (range.covers(b.range())){
			// b completely inside?
			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n), a);

		}else if ((i0 > bi0) and (i1 > bi1) and (i0 < bi1)){
			// overlapping end of b?
			addSubAction(new ActionTrack__ShrinkBufferBox(t, level_no, n, i0 - bi0), a);

		}else if ((i0 <= bi0) and (i1 < bi1) and (i1 > bi0)){
			// overlapping beginning of b?
			addSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i1 - bi0), a);
			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n), a);

		}else if (b.range().covers(range)){
			// inside b?
			addSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i1 - bi0), a);
			addSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i0 - bi0), a);
			addSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n + 1), a);

		}
		_foreach_it_.update();
	}
}

