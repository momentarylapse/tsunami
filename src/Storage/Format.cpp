/*
 * Format.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Format.h"
#include "../Tsunami.h"
#include "../Audio/AudioRenderer.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"

Format::Format(const string &_description, const string &_extensions, int _flags)
{
	description = _description;
	extensions = _extensions.explode(",");
	flags = _flags;
}

Format::~Format()
{
}

void Format::importData(Track *t, void *data, int channels, SampleFormat format, int samples, int offset, int level)
{
	msg_db_f("ImportData", 1);

	BufferBox buf = t->getBuffers(level, Range(offset, samples));

	Action *a = new ActionTrackEditBuffer(t, level, Range(offset, samples));
	buf.import(data, channels, format, samples);
	t->root->action_manager->execute(a);
}


bool Format::canHandle(const string & _extension)
{
	foreach(string e, extensions)
		if (e == _extension)
			return true;
	return false;
}

void Format::exportAudioAsTrack(AudioFile* a, const string& filename)
{
	BufferBox buf;
	AudioRenderer renderer;
	renderer.renderAudioFile(a, a->getRange(), buf);
	saveBuffer(a, &buf, filename);
}



bool Format::testFormatCompatibility(AudioFile *a)
{
	int num_subs = a->sample.num;
	int num_fx = a->fx.num;
	int num_audio = 0;
	int num_midi = 0;
	foreach(Track *t, a->track){
		num_fx += t->fx.num;
		if (t->type == t->TYPE_AUDIO)
			num_audio ++;
		if (t->type == t->TYPE_MIDI)
			num_midi ++;
	}

	if ((a->track.num > 1) && ((flags & Format::FLAG_MULTITRACK) == 0))
		return false;
	/*if ((a->tag.num > 0) && ((flags & StorageAny::FLAG_TAGS) == 0))
		return false;*/
	if ((num_fx > 0) && ((flags & Format::FLAG_FX) == 0))
		return false;
	if ((num_subs > 0) && ((flags & Format::FLAG_SUBS) == 0))
		return false;
	if ((num_audio > 0) && ((flags & Format::FLAG_AUDIO) == 0))
		return false;
	if ((num_midi > 0) && ((flags & Format::FLAG_MIDI) == 0))
		return false;
	return true;
}

