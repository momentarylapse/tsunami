/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionAudioDeleteSelection.h"
#include "../Track/ActionTrack__CutBufferBox.h"
#include "../Track/ActionTrack__DeleteBufferBox.h"
#include "../Track/ActionTrack__ShrinkBufferBox.h"
#include "../SubTrack/ActionSubTrackDelete.h"

ActionAudioDeleteSelection::ActionAudioDeleteSelection(AudioFile *a)
{
	int i0 = a->selection.start();
	int i1 = a->selection.end();
	foreachi(a->track, t, track_no)
		if (t.is_selected){
			// buffer boxes
			foreachi(t.level, l, li)
			foreachbi(l.buffer, b, n){
				int bi0 = b.offset;
				int bi1 = b.offset + b.num;


				if ((i0 <= bi0) && (i1 >= bi1)){
					// b completely inside?
					AddSubAction(new ActionTrack__DeleteBufferBox(&t, li, n), a);

				}else if ((i0 > bi0) && (i1 > bi1) && (i0 < bi1)){
					// overlapping end of b?
					AddSubAction(new ActionTrack__ShrinkBufferBox(&t, li, n, i0 - bi0), a);

				}else if ((i0 <= bi0) && (i1 < bi1) && (i1 > bi0)){
					// overlapping beginning of b?
					AddSubAction(new ActionTrack__CutBufferBox(&t, li, n, i1 - bi0), a);
					AddSubAction(new ActionTrack__DeleteBufferBox(&t, li, n), a);

				}else if ((i0 > bi0) && (i1 < bi1)){
					// inside b?
					AddSubAction(new ActionTrack__CutBufferBox(&t, li, n, i1 - bi0), a);
					AddSubAction(new ActionTrack__CutBufferBox(&t, li, n, i0 - bi0), a);
					AddSubAction(new ActionTrack__DeleteBufferBox(&t, li, n + 1), a);

				}
			}

			// subs
			foreachbi(t.sub, s, n)
				if (s.is_selected)
					AddSubAction(new ActionSubTrackDelete(track_no, n), a);
		}
}

ActionAudioDeleteSelection::~ActionAudioDeleteSelection()
{
}
