/*
 * ActionAudioDeleteTag.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include "ActionAudioDeleteTag.h"
#include <assert.h>

ActionAudioDeleteTag::ActionAudioDeleteTag(int _index)
{
	index = _index;
}

ActionAudioDeleteTag::~ActionAudioDeleteTag()
{
}

void *ActionAudioDeleteTag::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);
	assert(index < a->tags.num);

	old_tag = a->tags[index];
	a->tags.erase(index);

	return NULL;
}

void ActionAudioDeleteTag::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->tags.insert(old_tag, index);
}

