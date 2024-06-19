/*
 * ActionSongDeleteTag.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include "ActionTagDelete.h"

#include <assert.h>

namespace tsunami {

ActionTagDelete::ActionTagDelete(int _index) {
	index = _index;
}

ActionTagDelete::~ActionTagDelete() {
}

void *ActionTagDelete::execute(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->tags.num);

	old_tag = a->tags[index];
	a->tags.erase(index);

	return nullptr;
}

void ActionTagDelete::undo(Data *d) {
	Song *a = dynamic_cast<Song*>(d);

	a->tags.insert(old_tag, index);
}

}

