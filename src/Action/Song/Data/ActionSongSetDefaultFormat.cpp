/*
 * ActionSongSetDefaultFormat.cpp
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#include "ActionSongSetDefaultFormat.h"
#include "../../../Data/Song.h"

ActionSongSetDefaultFormat::ActionSongSetDefaultFormat(SampleFormat _format, int _compression)
{
	format = _format;
	compression = _compression;
}

void *ActionSongSetDefaultFormat::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	SampleFormat t1 = s->default_format;
	s->default_format = format;
	format = t1;

	int t2 = s->compression;
	s->compression = compression;
	compression = t2;

	return NULL;
}

void ActionSongSetDefaultFormat::undo(Data *d)
{
	execute(d);
}
