/*
 * Format.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Format.h"
#include "../../Tsunami.h"
#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "../../Audio/Source/SongRenderer.h"

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
		if (t->type == t->Type::AUDIO)
			num_audio ++;
		if (t->type == t->Type::MIDI)
			num_midi ++;
	}

	if ((a->tracks.num > 1) and ((flags & Flag::MULTITRACK) == 0))
		return false;
	/*if ((a->tag.num > 0) and ((flags & Flag::TAGS) == 0))
		return false;*/
	if ((num_fx > 0) and ((flags & Flag::FX) == 0))
		return false;
	if ((num_subs > 0) and ((flags & Flag::SAMPLES) == 0))
		return false;
	if ((num_audio > 0) and ((flags & Flag::AUDIO) == 0))
		return false;
	if ((num_midi > 0) and ((flags & Flag::MIDI) == 0))
		return false;
	return true;
}

Format::Format()
{
	song = NULL;
	od = NULL;
	f = NULL;
}

void Format::importData(Track *t, void *data, int channels, SampleFormat format, int samples, int offset, int layer)
{
	if (t->song->action_manager->isEnabled()){
		AudioBuffer buf = t->getBuffers(layer, Range(offset, samples));

		Action *a = new ActionTrackEditBuffer(t, layer, Range(offset, samples));
		buf.import(data, channels, format, samples);
		t->song->action_manager->execute(a);
	}else{
		if (t->layers[0].buffers.num == 0){
			AudioBuffer dummy;
			t->layers[0].buffers.add(dummy);
		}

		if (t->layers[0].buffers[0].length < offset + samples)
			t->layers[0].buffers[0].resize(offset + samples);
		AudioBuffer buf;
		buf.set_as_ref(t->layers[0].buffers[0], offset, samples);

		buf.import(data, channels, format, samples);
	}
}


void Format::loadSong(StorageOperationData *od)
{
	od->track = od->song->addTrack(Track::Type::AUDIO, 0);
	loadTrack(od);
}

void Format::saveSong(StorageOperationData* od)
{
	SongRenderer renderer(od->song);
	od->num_samples = renderer.getNumSamples();
	od->renderer = &renderer;
	saveViaRenderer(od);
}
