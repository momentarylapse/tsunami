/*
 * ActionLayerMerge.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionLayerMerge.h"

#include "../Track/Buffer/ActionTrack__DeleteBufferBox.h"
#include "../Track/Buffer/ActionTrackEditBuffer.h"
#include "../Track/Buffer/ActionTrackCreateBuffers.h"
#include "../../Data/Song.h"
#include "ActionLayer__Delete.h"

ActionLayerMerge::ActionLayerMerge(int _source, int _target)
{
	source = _source;
	target = _target;
}

void ActionLayerMerge::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	for (Track* t : s->tracks){
		TrackLayer &ls = t->layers[source];
		for (int i=ls.buffers.num-1; i>=0; i--){
			Range r = ls.buffers[i].range();

			addSubAction(new ActionTrackCreateBuffers(t, target, r), d);

			Action* a = new ActionTrackEditBuffer(t, target, r);
			BufferBox buf = t->readBuffers(target, r);
			buf.add(ls.buffers[i], 0, 1.0f, 0.0f);
			addSubAction(a, d);

			addSubAction(new ActionTrack__DeleteBufferBox(t, source, i), d);
		}
	}

	addSubAction(new ActionLayer__Delete(source), d);
}

