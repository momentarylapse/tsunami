/*
 * ActionAudioRenameLevel.cpp
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#include "ActionAudioRenameLevel.h"
#include "../../../Data/AudioFile.h"
#include <assert.h>

ActionAudioRenameLevel::ActionAudioRenameLevel(int _index, const string &_name)
{
	index = _index;
	name = _name;
}

ActionAudioRenameLevel::~ActionAudioRenameLevel()
{
}

void* ActionAudioRenameLevel::execute(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);
	assert(index < a->level_names.num);

	string temp = name;
	name = a->level_names[index];
	a->level_names[index] = temp;

	return NULL;
}

void ActionAudioRenameLevel::undo(Data* d)
{
	execute(d);
}

