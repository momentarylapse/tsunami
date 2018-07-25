/*
 * ActionBarEdit.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarEdit.h"
#include "../../Data/Track.h"
#include "../../Data/Song.h"
#include "../../Data/Rhythm/Bar.h"
#include "Action__ScaleData.h"
#include "ActionBar__Edit.h"
#include <assert.h>

ActionBarEdit::ActionBarEdit(int _index, int _length, int _num_beats, int _num_sub_beats, bool _affect_data)
{
	index = _index;
	length = _length;
	num_beats = _num_beats;
	num_sub_beats = _num_sub_beats;
	affect_data = _affect_data;
}

void ActionBarEdit::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	Range r = Range(s->barOffset(index), s->bars[index]->length);
	addSubAction(new ActionBar__Edit(index, length, num_beats, num_sub_beats), d);
	if (affect_data)
		addSubAction(new Action__ScaleData(r, length), d);
}

