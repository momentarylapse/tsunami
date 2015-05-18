/*
 * ActionAudioEditTag.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include "ActionAudioEditTag.h"
#include <assert.h>

ActionAudioEditTag::ActionAudioEditTag(int _index, const Tag &_tag)
{
	index = _index;
	new_tag = _tag;
}

ActionAudioEditTag::~ActionAudioEditTag()
{
}

void *ActionAudioEditTag::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);
	assert(index < a->tags.num);

	old_tag = a->tags[index];
	a->tags[index] = new_tag;

	return NULL;
}

void ActionAudioEditTag::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->tags[index] = old_tag;
}

