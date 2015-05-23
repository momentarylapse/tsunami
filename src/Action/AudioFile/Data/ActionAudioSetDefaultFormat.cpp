/*
 * ActionAudioSetDefaultFormat.cpp
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#include "ActionAudioSetDefaultFormat.h"
#include "../../../Data/AudioFile.h"

ActionAudioSetDefaultFormat::ActionAudioSetDefaultFormat(SampleFormat _format, int _compression)
{
	format = _format;
	compression = _compression;
}

void *ActionAudioSetDefaultFormat::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	SampleFormat t1 = a->default_format;
	a->default_format = format;
	format = t1;

	int t2 = a->compression;
	a->compression = compression;
	compression = t2;

	return NULL;
}

void ActionAudioSetDefaultFormat::undo(Data *d)
{
	execute(d);
}
