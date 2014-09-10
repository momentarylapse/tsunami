/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionAudioDeleteSelection.h"
#include "../Track/Buffer/ActionTrack__CutBufferBox.h"
#include "../Track/Buffer/ActionTrack__DeleteBufferBox.h"
#include "../Track/Buffer/ActionTrack__ShrinkBufferBox.h"
#include "../Track/Sample/ActionTrackDeleteSample.h"
#include "../Track/Midi/ActionTrackDeleteMidiNote.h"

ActionAudioDeleteSelection::ActionAudioDeleteSelection(AudioFile *a, int level_no, const Range &range, bool all_levels)
{
	foreachi(Track *t, a->track, track_no)
		if (t->is_selected){
			// buffer boxes
			if (all_levels){
				foreachi(TrackLevel &l, t->level, li)
					DeleteBuffersFromTrackLevel(a, t, l, range, li);
			}else{
				DeleteBuffersFromTrackLevel(a, t, t->level[level_no], range, level_no);
			}

			// subs
			foreachib(SampleRef *s, t->sample, i)
				if (s->is_selected){
					AddSubAction(new ActionTrackDeleteSample(t, i), a);
					_foreach_it_.update(); // TODO...
				}

			// midi
			foreachib(MidiNote &n, t->midi, i)
				if (range.is_inside(n.range.offset)){
					AddSubAction(new ActionTrackDeleteMidiNote(t, i), a);
					_foreach_it_.update(); // TODO...
				}
		}
}

ActionAudioDeleteSelection::~ActionAudioDeleteSelection()
{
}

void ActionAudioDeleteSelection::DeleteBuffersFromTrackLevel(AudioFile* a,
		Track *t, TrackLevel& l, const Range &range, int level_no)
{
	int i0 = range.start();
	int i1 = range.end();
	foreachib(BufferBox &b, l.buffer, n){
		int bi0 = b.offset;
		int bi1 = b.offset + b.num;


		if (range.covers(b.range())){
			// b completely inside?
			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n), a);

		}else if ((i0 > bi0) && (i1 > bi1) && (i0 < bi1)){
			// overlapping end of b?
			AddSubAction(new ActionTrack__ShrinkBufferBox(t, level_no, n, i0 - bi0), a);

		}else if ((i0 <= bi0) && (i1 < bi1) && (i1 > bi0)){
			// overlapping beginning of b?
			AddSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i1 - bi0), a);
			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n), a);

		}else if (b.range().covers(range)){
			// inside b?
			AddSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i1 - bi0), a);
			AddSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i0 - bi0), a);
			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n + 1), a);

		}
		_foreach_it_.update();
	}
}

