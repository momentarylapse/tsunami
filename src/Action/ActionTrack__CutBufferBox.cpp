/*
 * ActionTrack__CutBufferBox.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__CutBufferBox.h"

ActionTrack__CutBufferBox::ActionTrack__CutBufferBox(Track *t, int _index, int _offset)
{
	get_track_sub_index(t, track_no, sub_no);
	index = _index;
	offset = _offset;
}

ActionTrack__CutBufferBox::~ActionTrack__CutBufferBox()
{
}

void ActionTrack__CutBufferBox::redo(Data *d)
{
}



void ActionTrack__CutBufferBox::undo(Data *d)
{
}



void *ActionTrack__CutBufferBox::execute(Data *d)
{
}


