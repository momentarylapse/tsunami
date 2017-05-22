/*
 * ActionLayerMove.cpp
 *
 *  Created on: 21.05.2017
 *      Author: michi
 */

#include "ActionLayerMove.h"
#include "../../Data/Song.h"

ActionLayerMove::ActionLayerMove(int _source, int _target)
{
	source = _source;
	target = _target;
}


void* ActionLayerMove::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->layers.move(source, target);
	for (Track *t: a->tracks)
		t->layers.move(source, target);

	return NULL;
}

void ActionLayerMove::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->layers.move(target, source);
	for (Track *t: a->tracks)
		t->layers.move(target, source);
}

