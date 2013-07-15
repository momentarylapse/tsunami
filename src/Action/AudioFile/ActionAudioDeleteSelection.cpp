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

ActionAudioDeleteSelection::ActionAudioDeleteSelection(AudioFile *a, int level_no, bool all_levels)
{
	foreachi(Track *t, a->track, track_no)
		if (t->is_selected){
			// buffer boxes
			if (all_levels){
				foreachi(TrackLevel &l, t->level, li)
					DeleteBuffersFromTrackLevel(a, t, l, li);
			}else{
				DeleteBuffersFromTrackLevel(a, t, t->level[level_no], level_no);
			}

			// subs
			foreachib(SampleRef *s, t->sample, n)
				if (s->is_selected){
					AddSubAction(new ActionTrackDeleteSample(track_no, n), a);
					_foreach_it_.update(); // TODO...
				}
		}
}

ActionAudioDeleteSelection::~ActionAudioDeleteSelection()
{
}

void ActionAudioDeleteSelection::DeleteBuffersFromTrackLevel(AudioFile* a,
		Track *t, TrackLevel& l, int level_no)
{
	int i0 = a->selection.start();
	int i1 = a->selection.end();
	foreachib(BufferBox &b, l.buffer, n){
		int bi0 = b.offset;
		int bi1 = b.offset + b.num;


		if (a->selection.covers(b.range())){
			// b completely inside?
			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n), a);

		}else if ((i0 > bi0) && (i1 > bi1) && (i0 < bi1)){
			// overlapping end of b?
			AddSubAction(new ActionTrack__ShrinkBufferBox(t, level_no, n, i0 - bi0), a);

		}else if ((i0 <= bi0) && (i1 < bi1) && (i1 > bi0)){
			// overlapping beginning of b?
			AddSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i1 - bi0), a);
			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n), a);

		}else if (b.range().covers(a->selection)){
			// inside b?
			AddSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i1 - bi0), a);
			AddSubAction(new ActionTrack__CutBufferBox(t, level_no, n, i0 - bi0), a);
			AddSubAction(new ActionTrack__DeleteBufferBox(t, level_no, n + 1), a);

		}
		_foreach_it_.update();
	}
}

