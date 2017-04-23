/*
 * ActionBarEdit.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarEdit.h"

#include "../../Data/Track.h"
#include <assert.h>

#include "Action__ScaleData.h"
#include "ActionBar__Edit.h"

ActionBarEdit::ActionBarEdit(int _index, BarPattern &_bar, bool _affect_data) :
	bar(_bar)
{
	index = _index;
	affect_data = _affect_data;
}

void ActionBarEdit::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	Range r = Range(s->barOffset(index), s->bars[index].length);
	addSubAction(new ActionBar__Edit(index, bar), d);
	if (affect_data)
		addSubAction(new Action__ScaleData(r, bar.length), d);
}

