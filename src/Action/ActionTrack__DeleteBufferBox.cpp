/*
 * ActionTrack__DeleteBufferBox.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__DeleteBufferBox.h"

ActionTrack__DeleteBufferBox::ActionTrack__DeleteBufferBox(Track *t, int _index)
{
	get_track_sub_index(t, track_no, sub_no);
	index = _index;
}

ActionTrack__DeleteBufferBox::~ActionTrack__DeleteBufferBox()
{
}

void ActionTrack__DeleteBufferBox::redo(Data *d)
{
}



void ActionTrack__DeleteBufferBox::undo(Data *d)
{
}



void *ActionTrack__DeleteBufferBox::execute(Data *d)
{
}


