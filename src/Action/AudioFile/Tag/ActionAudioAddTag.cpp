/*
 * ActionAudioAddTag.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include "ActionAudioAddTag.h"

ActionAudioAddTag::ActionAudioAddTag(const Tag &_tag)
{
	tag = _tag;
}

ActionAudioAddTag::~ActionAudioAddTag()
{
}

void *ActionAudioAddTag::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->tags.add(tag);

	return NULL;
}

void ActionAudioAddTag::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->tags.pop();
}

