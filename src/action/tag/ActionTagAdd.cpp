/*
 * ActionTagAdd.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include "ActionTagAdd.h"

namespace tsunami {

ActionTagAdd::ActionTagAdd(const Tag &_tag)
{
	tag = _tag;
}

ActionTagAdd::~ActionTagAdd()
{
}

void *ActionTagAdd::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->tags.add(tag);

	return nullptr;
}

void ActionTagAdd::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->tags.pop();
}

}

