/*
 * Format.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Format.h"
#include "../../Tsunami.h"
#include "../../Audio/Renderer/SongRenderer.h"
#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"

FormatDescriptor::FormatDescriptor(const string &_description, const string &_extensions, int _flags)
{
	description = _description;
	extensions = _extensions.explode(",");
	flags = _flags;
}

bool FormatDescriptor::canHandle(const string & _extension)
{
	for (string &e : extensions)
		if (e == _extension)
			return true;
	return false;
}

bool FormatDescriptor::testFormatCompatibility(Song *a)
{
	int num_subs = a->samples.num;
	int num_fx = a->fx.num;
	int num_audio = 0;
	int num_midi = 0;
	for (Track *t : a->tracks){
		num_fx += t->fx.num;
		if (t->type == t->TYPE_AUDIO)
			num_audio ++;
		if (t->type == t->TYPE_MIDI)
			num_midi ++;
	}

	if ((a->tracks.num > 1) && ((flags & FLAG_MULTITRACK) == 0))
		return false;
	/*if ((a->tag.num > 0) && ((flags & FLAG_TAGS) == 0))
		return false;*/
	if ((num_fx > 0) && ((flags & FLAG_FX) == 0))
		return false;
	if ((num_subs > 0) && ((flags & FLAG_SUBS) == 0))
		return false;
	if ((num_audio > 0) && ((flags & FLAG_AUDIO) == 0))
		return false;
	if ((num_midi > 0) && ((flags & FLAG_MIDI) == 0))
		return false;
	return true;
}

Format::Format()
{
	song = NULL;
	od = NULL;
	f = NULL;
}

void Format::importData(Track *t, void *data, int channels, SampleFormat format, int samples, int offset, int level)
{
	msg_db_f("ImportData", 1);

	BufferBox buf = t->getBuffers(level, Range(offset, samples));

	Action *a;
	if (t->song->action_manager->isEnabled())
		a = new ActionTrackEditBuffer(t, level, Range(offset, samples));
	buf.import(data, channels, format, samples);
	if (t->song->action_manager->isEnabled())
		t->song->action_manager->execute(a);
}

/*void Format::exportAsTrack(StorageOperationData *od)
{
	SongRenderer renderer;

	const int CHUNK_SIZE = 1<<16;

	Range r = od->song->getRange();
	od->samples = r.num;
	saveBufferBegin(od);
	int written = 0;
	while(written < r.num){

		BufferBox buf;
		renderer.render(od->song, Range(r.offset + written, CHUNK_SIZE), buf);
		od->buf = &buf;
		saveBufferPart(od);
		written += CHUNK_SIZE;
	}
	saveBufferEnd(od);
}*/



void Format::loadSong(StorageOperationData *od)
{
	od->track = od->song->addTrack(Track::TYPE_AUDIO, 0);
	loadTrack(od);
}

void Format::saveSong(StorageOperationData* od)
{
	SongRenderer renderer(od->song, NULL);
	od->renderer = &renderer;
	saveViaRenderer(od);
}
