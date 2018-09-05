/*
 * TrackRenderer.cpp
 *
 *  Created on: 02.09.2018
 *      Author: michi
 */

#include "TrackRenderer.h"
#include "SongRenderer.h"
#include "AudioEffect.h"
#include "../Beats/BarStreamer.h"
#include "../Beats/BeatMidifier.h"
#include "../Midi/MidiEffect.h"
#include "../Synth/Synthesizer.h"
#include "../Midi/MidiEventStreamer.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/Sample.h"
#include "../../Data/SampleRef.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../lib/math/math.h"
#include "../../Session.h"



bool intersect_sub(SampleRef *s, const Range &r, Range &ir, int &bpos)
{
	// intersected intervall (track-coordinates)
	int i0 = max(s->pos, r.start());
	int i1 = min(s->pos + s->buf->length, r.end());

	// beginning of the intervall (relative to sub)
	ir.offset = i0 - s->pos;
	// ~ (relative to old intervall)
	bpos = i0 - r.start();
	ir.length = i1 - i0;

	return !ir.empty();
}


int get_first_usable_layer(Track *t, Set<TrackLayer*> &allowed)
{
	foreachi(TrackLayer *l, t->layers, i)
		if (allowed.contains(l))
			return i;
	return -1;
}

static void add_samples(TrackLayer *l, const Range &range_cur, AudioBuffer &buf)
{
	// subs
	for (SampleRef *s: l->samples){
		if (s->muted)
			continue;

		Range intersect_range;
		int bpos;
		if (!intersect_sub(s, range_cur, intersect_range, bpos))
			continue;

		bpos = s->pos - range_cur.start();
		buf.add(*s->buf, bpos, s->volume * s->origin->volume, 0);
	}
}

void TrackRenderer::reset_state()
{
	for (AudioEffect *fx: fx)
		fx->reset_state();
	synth->reset();
}

TrackRenderer::TrackRenderer(Track *t, SongRenderer *sr)
{
	song_renderer = sr;
	track = t;
	if (song_renderer->direct_mode)
		synth = t->synth;
	else
		synth = (Synthesizer*)t->synth->copy();
	synth->setSampleRate(t->song->sample_rate);
	synth->setInstrument(t->instrument);
	midi_streamer = nullptr;

	if (song_renderer->direct_mode){
		fx = t->fx;
	}else{
		for (AudioEffect *f: t->fx)
			fx.add((AudioEffect*)f->copy());
	}

	//midi.add(t, t->midi);
	if (t->type == SignalType::MIDI){
		MidiNoteBuffer _midi = t->layers[0]->midi;
		for (TrackLayer *l: t->layers)
			for (auto c: l->samples)
				if (c->type() == SignalType::MIDI)
				_midi.append(*c->midi, c->pos); // TODO: mute/solo....argh
		for (MidiEffect *fx: t->midi_fx){
			fx->prepare();
			fx->process(&_midi);
		}

		MidiEventBuffer raw = midi_notes_to_events(_midi);
		midi_streamer = new MidiEventStreamer(raw);
		midi_streamer->ignore_end = true;

		synth->set_source(midi_streamer->out);
	}else if (t->type == SignalType::BEATS){

		synth->set_source(sr->beat_midifier->out);
	}

	track->subscribe(this, [&]{ on_track_replace_synth(); }, track->MESSAGE_REPLACE_SYNTHESIZER);
	track->subscribe(this, [&]{ on_track_add_or_delete_fx(); }, track->MESSAGE_ADD_EFFECT);
	track->subscribe(this, [&]{ on_track_add_or_delete_fx(); }, track->MESSAGE_DELETE_EFFECT);
	track->subscribe(this, [&]{ on_track_change_data(); }, track->MESSAGE_CHANGE);
}

TrackRenderer::~TrackRenderer()
{
	track->unsubscribe(this);
	if (midi_streamer)
		delete midi_streamer;
	if (!song_renderer->direct_mode){
		for (auto *f: fx)
			delete f;
		delete synth;
	}
}

void TrackRenderer::on_track_add_or_delete_fx()
{
	msg_error("track del/rm fx");
}

void TrackRenderer::on_track_replace_synth()
{
	if (song_renderer->direct_mode){
		synth = track->synth;
	}else{
		delete synth;

		synth = (Synthesizer*)track->synth->copy();
		synth->setSampleRate(track->song->sample_rate);
		synth->setInstrument(track->instrument);
	}

	if (track->type == SignalType::MIDI){
		synth->set_source(midi_streamer->out);
	}else if (track->type == SignalType::BEATS){
		synth->set_source(song_renderer->beat_midifier->out);
	}
}

void TrackRenderer::on_track_change_data()
{
	if (midi_streamer){
		MidiNoteBuffer _midi = track->layers[0]->midi;
		for (TrackLayer *l: track->layers)
			for (auto c: l->samples)
				if (c->type() == SignalType::MIDI)
				_midi.append(*c->midi, c->pos); // TODO: mute/solo....argh
		midi_streamer->midi = _midi.getEvents(Range::ALL);
	}
}

void TrackRenderer::seek(int pos)
{
	if (midi_streamer)
		midi_streamer->seek(pos);
	if (track->type == SignalType::BEATS)
		song_renderer->bar_streamer->seek(pos);
}


void TrackRenderer::render_audio_no_fx(AudioBuffer &buf)
{
	/*bool has_solo = false;
	for (auto *l: track->layers)
		if (l->)*/

	Range &cur = song_renderer->range_cur;

	int index_before = 0;
	Track::Fade *ff = nullptr;
	foreachi(auto &f, track->fades, i){
		Range r = f.range();
		if (r.end() <= song_renderer->range_cur.start())
			index_before = f.target;
		if (r.overlaps(song_renderer->range_cur)){
			if (ff)
				song_renderer->session->e("SongRenderer: double fade...");
			ff = &f;
		}
	}
	if (ff){
		Range r = ff->range();
		// influence of previous layer
		Range r1 = RangeTo(cur.offset, r.end()) and cur;
		AudioBuffer tbuf1;
		tbuf1.set_as_ref(buf, 0, r1.length);
		track->layers[index_before]->read_buffers_fixed(tbuf1, r1);


		// influence of target layer
		Range r2 = RangeTo(r.start(), cur.end()) and cur;
		AudioBuffer tbuf2;
		tbuf2.resize(r2.length);
		track->layers[ff->target]->read_buffers_fixed(tbuf2, r2);

		// perform (linear) fade
		Range r3 = r and cur;
		for (int i=r3.start(); i<r3.end(); i++){
			float a = 1 - (float)(i - r.start()) / (float)ff->samples;
			for (int c=0; c<buf.channels; c++)
				buf.c[c][i - r1.start()] *= a;
		}
		for (int i=r3.start(); i<r3.end(); i++){
			float a = 1 - (float)(r.end() - i) / (float)ff->samples;
			for (int c=0; c<buf.channels; c++)
				tbuf2.c[c][i - r2.start()] *= a;
		}

		buf.add(tbuf2, r2.start() - r1.start(), 1, 0);

	}else{
		// direct mode
		track->layers[index_before]->read_buffers_fixed(buf, song_renderer->range_cur);
		add_samples(track->layers[index_before], song_renderer->range_cur, buf);
	}

		/*
		return;
	//}

	// any un-muted layer?
	int i0 = get_first_usable_layer(track, song_renderer->allowed_layers);
	if (i0 < 0){
		// no -> return silence
		buf.scale(0);
	}else{

		// first (un-muted) layer
		track->layers[i0]->read_buffers_fixed(buf, song_renderer->range_cur);
		// TODO: allow_ref if no other layers + no fx
		add_samples(track->layers[i0], song_renderer->range_cur, buf);

		// other layers
		AudioBuffer tbuf;
		for (int i=i0+1;i<track->layers.num;i++){
			if (!song_renderer->allowed_layers.contains(track->layers[i]))
				continue;
			if (track->layers[i]->muted)
				continue;
			track->layers[i]->readBuffers(tbuf, song_renderer->range_cur, true);
			add_samples(track->layers[i], song_renderer->range_cur, tbuf);
			buf.add(tbuf, 0, 1.0f, 0.0f);
		}
	}*/
}

void TrackRenderer::render_time_no_fx(AudioBuffer &buf)
{
	synth->out->read(buf);
}

void TrackRenderer::render_midi_no_fx(AudioBuffer &buf)
{
	synth->out->read(buf);
}

void TrackRenderer::render_no_fx(AudioBuffer &buf)
{
	if (track->type == SignalType::AUDIO)
		render_audio_no_fx(buf);
	else if (track->type == SignalType::BEATS)
		render_time_no_fx(buf);
	else if (track->type == SignalType::MIDI)
		render_midi_no_fx(buf);
}

void TrackRenderer::apply_fx(AudioBuffer &buf, Array<AudioEffect*> &fx_list)
{
	// apply fx
	for (AudioEffect *f: fx_list)
		if (f->enabled){
			f->process(buf);
		}
}

void TrackRenderer::render_fx(AudioBuffer &buf)
{
	render_no_fx(buf);

	Array<AudioEffect*> _fx = fx;
	if (song_renderer->preview_effect)
		_fx.add(song_renderer->preview_effect);
	if (_fx.num > 0)
		apply_fx(buf, _fx);
}
