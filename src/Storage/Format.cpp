/*
 * Format.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Format.h"
#include "../Tsunami.h"
#include "../Audio/AudioRenderer.h"

Format::Format(const string &_extension, int _flags)
{
	extension = _extension;
	flags = _flags;
}

Format::~Format()
{
}

void Format::ImportData(Track *t, void *data, int channels, int bits, int samples, int offset)
{
	msg_db_r("ImportData", 1);

	BufferBox buf = t->GetBuffers(0, Range(offset, samples));
	buf.import(data, channels, bits, samples);

	msg_db_l(1);
}


bool Format::CanHandle(const string & _extension)
{
	return (extension == _extension);
}

void Format::ExportAudioAsTrack(AudioFile* a, const string& filename)
{
	BufferBox buf = tsunami->renderer->RenderAudioFile(a, a->GetRange());
	SaveBuffer(a, &buf, filename);
}



bool Format::TestFormatCompatibility(AudioFile *a)
{
	int num_subs = 0;
	int num_fx = a->fx.num;
	foreach(Track *t, a->track){
		num_subs += t->sub.num;
		num_fx += t->fx.num;
	}

	if ((a->track.num > 1) && ((flags & Format::FLAG_MULTITRACK) == 0))
		return false;
	/*if ((a->tag.num > 0) && ((flags & StorageAny::FLAG_TAGS) == 0))
		return false;*/
	if ((num_fx > 0) && ((flags & Format::FLAG_FX) == 0))
		return false;
	if ((num_subs > 0) && ((flags & Format::FLAG_SUBS) == 0))
		return false;
	return true;
}

