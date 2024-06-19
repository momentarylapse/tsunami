/*
 * Diff.cpp
 *
 *  Created on: Sep 28, 2020
 *      Author: michi
 */

#include "Diff.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/Sample.h"
#include "../../data/CrossFade.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../data/rhythm/Bar.h"
#include "../../module/audio/AudioEffect.h"
#include "../../module/midi/MidiEffect.h"
#include "../../module/synthesizer/Synthesizer.h"

namespace tsunami {

Array<string> diff_module(Module *a, Module *b, const string &ee) {
	Array<string> r;
	if (a->module_class != b->module_class)
		r.add(ee + "type");
	if (a->config_to_string() != b->config_to_string())
		r.add(ee + "config");
	return r;
}

Array<string> diff_buffer(AudioBuffer *a, AudioBuffer *b, const string &ee) {
	Array<string> r;
	if (a->offset != b->offset)
		r.add(ee + "offset");
	if (a->channels != b->channels)
		r.add(ee + "channels");
	if (a->length != b->length)
		r.add(ee + "length");
	for (int i=0; i<a->channels; i++)
		if (a->c[i] != b->c[i])
			r.add(ee + "data");
	return r;
}

Array<string> diff_midi(const MidiNoteBuffer &a, const MidiNoteBuffer &b, const string &ee) {
	Array<string> r;
	if (a.num != b.num)
		r.add(ee + "#");
	bool err_pitch = false;
	bool err_volume = false;
	bool err_range = false;
	bool err_flags = false;
	bool err_modifier = false;
	bool err_string = false;
	for (int i=0; i<min(a.num, b.num); i++) {
		if (a[i]->pitch != b[i]->pitch)
			err_pitch = true;
		if (a[i]->volume != b[i]->volume)
			err_volume = true;
		if (a[i]->range != b[i]->range)
			err_range = true;
		if (a[i]->flags != b[i]->flags)
			err_flags = true;
		if (a[i]->modifier != b[i]->modifier)
			err_modifier = true;
		if (a[i]->stringno != b[i]->stringno)
			err_string = true;
	}
	if (err_pitch)
		r.add(ee + "pitch");
	if (err_volume)
		r.add(ee + "volume");
	if (err_range)
		r.add(ee + "range");
	if (err_flags)
		r.add(ee + "flags");
	if (err_modifier)
		r.add(ee + "modifier");
	if (err_string)
		r.add(ee + "string");
	return r;
}

Array<string> diff_sample(Sample *a, Sample *b, const string &ee) {
	Array<string> r;
	if (a->type != b->type)
		r.add(ee + "type");
	if (a->name != b->name)
		r.add(ee + "name");
	if (a->uid != b->uid)
		r.add(ee + "uid");
	if (a->tags != b->tags)
		r.add(ee + "tags");
	if (a->buf and b->buf)
		r += diff_buffer(a->buf, b->buf, ee + "buffer.");
	r += diff_midi(a->midi, b->midi, ee + "midi.");
	return r;
}

Array<string> diff_layer(TrackLayer *la, TrackLayer *lb, const string &ee) {
	Array<string> r;
	if (la->muted != lb->muted)
		r.add(ee + "muted");
	if (la->fades != lb->fades)
		r.add(ee + "fades");
	if (la->markers.num != lb->markers.num)
		r.add(ee + "#markers");
	if (la->buffers.num != lb->buffers.num)
		r.add(ee + "#buffers");
	for (int i=0; i<min(la->buffers.num, lb->buffers.num); i++)
		r += diff_buffer(&la->buffers[i], &lb->buffers[i], ee + format("buffer[%d]", i));
	r += diff_midi(la->midi, lb->midi, ee + "midi.");
	return r;
}

Array<string> diff_track(Track *ta, Track *tb, const string &ee) {
	Array<string> r;
	if (ta->name != tb->name)
		r.add(ee + "name");
	if (ta->volume != tb->volume)
		r.add(ee + "volume");
	if (ta->channels != tb->channels)
		r.add(ee + "channels");
	if (ta->instrument != tb->instrument)
		r.add(ee + "instrument");
	if (ta->muted != tb->muted)
		r.add(ee + "muted");
	if (ta->panning != tb->panning)
		r.add(ee + "panning");

	if (ta->curves.num != tb->curves.num)
		r.add("#curves");

	r += diff_module(ta->synth.get(), tb->synth.get(), ee + "synth.");

	if (ta->fx.num != tb->fx.num)
		r.add(ee + "#fx");
	for (int i=0; i<min(ta->fx.num, tb->fx.num); i++)
		r += diff_module(ta->fx[i].get(), tb->fx[i].get(), ee  + format("fx[%d].", i));

	if (ta->midi_fx.num != tb->midi_fx.num)
		r.add(ee + "#midi_fx");
	for (int i=0; i<min(ta->midi_fx.num, tb->midi_fx.num); i++)
		r += diff_module(ta->midi_fx[i].get(), tb->midi_fx[i].get(), ee  + format("midi_fx[%d].", i));

	if (ta->layers.num != tb->layers.num)
		r.add(ee + "#layers");
	for (int i=0; i<min(ta->layers.num, tb->layers.num); i++)
		r += diff_layer(ta->layers[i].get(), tb->layers[i].get(), ee + format("layer[%d].", i));
	return r;
}


Array<string> diff_song(Song *a, Song *b) {
	Array<string> r;
	if (a->bars.num != b->bars.num)
		r.add("#bars");
	for (int i=0; i<min(a->bars.num, b->bars.num); i++)
		if (*(BarPattern*)a->bars[i].get() != *(BarPattern*)b->bars[i].get())
			r.add(format("bar[%d]", i));

	if (a->tags != b->tags)
		r.add("tags");
	if (a->sample_rate != b->sample_rate)
		r.add("sample_rate");
	if (a->secret_data.str() != b->secret_data.str())
		r.add("secret");

	if (a->samples.num != b->samples.num)
		r.add("#samples");
	for (int i=0; i<min(a->samples.num, b->samples.num); i++)
		r += diff_sample(a->samples[i].get(), b->samples[i].get(), format("sample[i].", i));

	if (a->tracks.num != b->tracks.num)
		r.add("#tracks");
	for (int i=0; i<min(a->tracks.num, b->tracks.num); i++)
		r += diff_track(a->tracks[i].get(), b->tracks[i].get(), format("track[%d].", i));
	return r;
}

}
