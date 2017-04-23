/*
 * ActionLayerRename.cpp
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#include "ActionLayerRename.h"

#include <assert.h>
#include "../../Data/Song.h"

ActionLayerRename::ActionLayerRename(int _index, const string &_name)
{
	index = _index;
	name = _name;
}

ActionLayerRename::~ActionLayerRename()
{
}

void* ActionLayerRename::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->layer_names.num);

	string temp = name;
	name = a->layer_names[index];
	a->layer_names[index] = temp;

	a->notify(a->MESSAGE_EDIT_LAYER);

	return NULL;
}

void ActionLayerRename::undo(Data* d)
{
	execute(d);
}

