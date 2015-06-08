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
#include "../Track/Midi/ActionTrackDeleteMidiEvent.h"
#include "../Track/Sample/ActionTrackDeleteSample.h"

ActionAudioDeleteSelection::ActionAudioDeleteSelection(AudioFile *a, int level_no, const Range &range, bool all_levels)
{
	foreachi(Track *t, a->tracks, track_no)
		if (t->is_selected){
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
			Array<int> note_events[128];
			foreachi(MidiEvent &e, t->midi, i){
				int pitch = (int)e.pitch;
				note_events[pitch].add(i);
				if (e.volume <= 0){
					int p0 = t->midi[note_events[pitch][0]].pos;
					int p1 = t->midi[note_events[pitch].back()].pos;
					Range r = Range(p0, p1 - p0);
					if (r.overlaps(range)){
						foreach(int n, note_events[pitch])
						to_delete.add(n);
					}
					note_events[pitch].clear();
				}
			}

			foreachb(int n, to_delete)
				addSubAction(new ActionTrackDeleteMidiEvent(t, n), a);
		}
}

void ActionAudioDeleteSelection::DeleteBuffersFromTrackLevel(AudioFile* a,
		Track *t, TrackLevel& l, const Range &range, int level_no)
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

