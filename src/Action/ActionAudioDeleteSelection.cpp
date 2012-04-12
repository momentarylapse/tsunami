/*
 * ActionAudioDeleteSelection.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionAudioDeleteSelection.h"
#include "ActionTrack__CutBufferBox.h"
#include "ActionTrack__DeleteBufferBox.h"
#include "ActionTrack__ShrinkBufferBox.h"

ActionAudioDeleteSelection::ActionAudioDeleteSelection(AudioFile *a)
{
	int i0 = a->selection.start();
	int i1 = a->selection.end();
	foreach(a->track, t)
		if (t.is_selected){
			// buffer boxes
			foreachbi(t.buffer, b, n){
				int bi0 = b.offset;
				int bi1 = b.offset + b.num;


				if ((i0 <= bi0) && (i1 >= bi1)){
					// b completely inside?
					AddSubAction(new ActionTrack__DeleteBufferBox(&t, n), a);

				}else if ((i0 > bi0) && (i1 > bi1) && (i0 < bi1)){
					// overlapping end of b?
					AddSubAction(new ActionTrack__ShrinkBufferBox(&t, n, i0 - bi0), a);

				}else if ((i0 <= bi0) && (i1 < bi1) && (i1 > bi0)){
					// overlapping beginning of b?
					AddSubAction(new ActionTrack__CutBufferBox(&t, n, i1 - bi0), a);
					AddSubAction(new ActionTrack__DeleteBufferBox(&t, n), a);

				}else if ((i0 > bi0) && (i1 < bi1)){
					// inside b?
					AddSubAction(new ActionTrack__CutBufferBox(&t, n, i1 - bi0), a);
					AddSubAction(new ActionTrack__CutBufferBox(&t, n, i0 - bi0), a);
					AddSubAction(new ActionTrack__DeleteBufferBox(&t, n + 1), a);

				}
			}
		}
}

ActionAudioDeleteSelection::~ActionAudioDeleteSelection()
{
}
