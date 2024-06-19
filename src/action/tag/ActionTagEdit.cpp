/*
 * ActionTagEdit.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include "ActionTagEdit.h"

#include <assert.h>

namespace tsunami {

ActionTagEdit::ActionTagEdit(int _index, const Tag &_tag) {
	index = _index;
	new_tag = _tag;
}

void *ActionTagEdit::execute(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->tags.num);

	old_tag = a->tags[index];
	a->tags[index] = new_tag;

	return nullptr;
}

void ActionTagEdit::undo(Data *d) {
	Song *a = dynamic_cast<Song*>(d);

	a->tags[index] = old_tag;
}

}

