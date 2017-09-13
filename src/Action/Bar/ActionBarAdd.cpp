/*
 * ActionBarAdd.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarAdd.h"

#include "../../Data/Track.h"
#include <assert.h>

#include "../Track/Buffer/ActionTrack__SplitBuffer.h"
#include "Action__ShiftData.h"
#include "ActionBar__Add.h"

ActionBarAdd::ActionBarAdd(int _index, BarPattern &_bar, bool _affect_data)
{
	index = _index;
	bar = _bar;
	affect_data = _affect_data;
}

void ActionBarAdd::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	addSubAction(new ActionBar__Add(index, bar), d);

	if (affect_data){
		int pos0 = s->barOffset(index);

		for (Track *t: s->tracks)
			for (int l=0; l<t->layers.num; l++)
				for (int i=t->layers[l].buffers.num-1; i>=0; i--)
					if (t->layers[l].buffers[i].range().is_more_inside(pos0))
						addSubAction(new ActionTrack__SplitBuffer(t, l, i, pos0 - t->layers[l].buffers[i].offset), d);

		addSubAction(new Action__ShiftData(pos0, bar.length), d);

	}
}

