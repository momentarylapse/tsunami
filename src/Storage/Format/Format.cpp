/*
 * Format.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "Format.h"
#include "../../Tsunami.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Action/ActionManager.h"
#include "../../Module/Audio/SongRenderer.h"

FormatDescriptor::FormatDescriptor(const string &_description, const string &_extensions, int _flags) {
	description = _description;
	extensions = _extensions.explode(",");
	flags = _flags;
}

bool FormatDescriptor::can_handle(const string & _extension) {
	for (string &e: extensions)
		if (e == _extension)
			return true;
	return false;
}

bool FormatDescriptor::test_compatibility(Song *a) {
	int num_subs = a->samples.num;
	int num_fx = 0;
	int num_audio = 0;
	int num_midi = 0;
	for (Track *t: a->tracks) {
		num_fx += t->fx.num;
		if (t->type == SignalType::AUDIO)
			num_audio ++;
		if (t->type == SignalType::MIDI)
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

Format::Format() {
	f = nullptr;
}

void Format::import_data(TrackLayer *layer, void *data, int channels, SampleFormat format, int samples, int offset) {
	AudioBuffer buf;
	auto *a = layer->edit_buffers(buf, Range(offset, samples));

	buf.import(data, channels, format, samples);
	layer->edit_buffers_finish(a);
}


void Format::load_song(StorageOperationData *od) {
	od->track = od->song->add_track(SignalType::AUDIO_STEREO, 0);
	od->layer = od->track->layers[0];
	od->allow_channels_change = true;
	load_track(od);
}

bool is_simple_track(Song *s) {
	if (s->tracks.num > 1)
		return false;
	if (s->tracks[0]->type != SignalType::AUDIO)
		return false;
	if (s->tracks[0]->fx.num > 0)
		return false;
	if (s->tracks[0]->layers.num > 1)
		return false;
	return true;
}

void Format::save_song(StorageOperationData* od) {
	SongRenderer renderer(od->song);

	if (is_simple_track(od->song)) {
		od->channels_suggested = od->song->tracks[0]->channels;
	}

	od->num_samples = renderer.get_num_samples();
	od->renderer = renderer.out;
	save_via_renderer(od);
}
