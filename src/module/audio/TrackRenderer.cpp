/*
 * TrackRenderer.cpp
 *
 *  Created on: 02.09.2018
 *      Author: michi
 */

#include "TrackRenderer.h"
#include "SongRenderer.h"
#include "AudioEffect.h"
#include "../beats/BarStreamer.h"
#include "../beats/BeatMidifier.h"
#include "../midi/MidiEffect.h"
#include "../synthesizer/Synthesizer.h"
#include "../midi/MidiEventStreamer.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/Curve.h"
#include "../../data/CrossFade.h"
#include "../../data/Sample.h"
#include "../../data/SampleRef.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/math/math.h"
#include "../../Session.h"

#include "../../lib/os/msg.h"


const int CURVE_CHUNK = 512;


bool intersect_sub(SampleRef *s, const Range &r, Range &ir, int &bpos) {
	// intersected intervall (track-coordinates)
	int i0 = max(s->pos, r.start());
	int i1 = min(s->pos + s->buf().length, r.end());

	// beginning of the intervall (relative to sub)
	ir.offset = i0 - s->pos;
	// ~ (relative to old intervall)
	bpos = i0 - r.start();
	ir.length = i1 - i0;

	return !ir.is_empty();
}


int TrackRenderer::get_first_usable_layer() {
	if (!song_renderer)
		return 0;
	foreachi(TrackLayer *l, weak(track->layers), i) {
		if (song_renderer->allowed_layers.contains(l))
			return i;
	}
	return -1;
}

static void add_samples(TrackLayer *l, const Range &range_cur, AudioBuffer &buf) {
	// subs
	for (SampleRef *s: weak(l->samples)) {
		if (s->muted)
			continue;

		Range intersect_range;
		int bpos;
		if (!intersect_sub(s, range_cur, intersect_range, bpos))
			continue;

		bpos = s->pos - range_cur.start();
		buf.add(s->buf(), bpos, s->volume * s->origin->volume);
	}
}

void TrackRenderer::reset_state() {
	if (direct_mode)
		fx = track->fx;
	if (midi_streamer)
		fill_midi_streamer();

	for (AudioEffect *fx: weak(fx))
		fx->reset_state();
	synth->reset_state();
}

TrackRenderer::TrackRenderer(Track *t, SongRenderer *sr) {
	module_class = "TrackRenderer";
	song_renderer = sr;
	track = t;
	offset = 0;
	peak[0] = peak[1] = 0;
	direct_mode = song_renderer and song_renderer->direct_mode;
	if (direct_mode)
		synth = t->synth;
	else
		synth = (Synthesizer*)t->synth->copy();
	synth->set_sample_rate(t->song->sample_rate);
	synth->set_instrument(t->instrument);

	if (direct_mode) {
		fx = t->fx;
	} else {
		for (AudioEffect *f: weak(t->fx))
			fx.add((AudioEffect*)f->copy());
	}

	//midi.add(t, t->midi);
	if (t->type == SignalType::MIDI) {
		MidiEventBuffer raw;
		midi_streamer = new MidiEventStreamer(raw);
		midi_streamer->perf_set_parent(this);
		synth->_plug_in(0, midi_streamer.get(), 0);
		fill_midi_streamer();
	} else if (t->type == SignalType::BEATS) {
		beat_midifier = new BeatMidifier;
		beat_midifier->_plug_in(0, sr->bar_streamer.get(), 0);
		beat_midifier->perf_set_parent(this);
		synth->_plug_in(0, beat_midifier.get(), 0);
	}
	
	perf_set_parent(song_renderer);
	synth->perf_set_parent(this);
	
	for (auto *f: weak(fx))
		f->perf_set_parent(this);
		

	track->subscribe(this, [=]{ on_track_replace_synth(); }, track->MESSAGE_REPLACE_SYNTHESIZER);
	track->subscribe(this, [=]{ on_track_add_or_delete_fx(); }, track->MESSAGE_ADD_EFFECT);
	track->subscribe(this, [=]{ on_track_add_or_delete_fx(); }, track->MESSAGE_DELETE_EFFECT);
	track->subscribe(this, [=]{ on_track_change_data(); }, track->MESSAGE_CHANGE);
	track->subscribe(this, [=]{ on_track_delete(); }, track->MESSAGE_DELETE);
	track->song->subscribe(this, [=]{ update_layers(); }, track->song->MESSAGE_ADD_LAYER);
	track->song->subscribe(this, [=]{ update_layers(); }, track->song->MESSAGE_DELETE_LAYER);

	update_layers();
}

TrackRenderer::~TrackRenderer() {
	unlink_from_track();

	if (synth)
		synth->perf_set_parent(nullptr);
	for (auto *l: weak(layers))
		l->unsubscribe(this);
	if (track) {
		track->song->unsubscribe(this);
		track->unsubscribe(this);
	}
}

void TrackRenderer::unlink_from_track() {
	if (synth) {
		synth->perf_set_parent(nullptr);
		synth = nullptr;
	}

	if (track) {
		track->song->unsubscribe(this);
		track->unsubscribe(this);
		track = nullptr;
	}

	if (!direct_mode)
		fx.clear();

	update_layers();
}

void TrackRenderer::on_track_delete() {
	unlink_from_track();
}

void TrackRenderer::fill_midi_streamer() {
	if (!track)
		return;
	MidiNoteBuffer _midi;
	if (layers.num > 0)
		_midi = layers[0]->midi;
	for (auto *l: weak(layers))
		for (auto c: weak(l->samples))
			if (c->type() == SignalType::MIDI)
			_midi.append(c->midi(), c->pos); // TODO: mute/solo....argh

	MidiEventBuffer events = midi_notes_to_events(_midi);

	for (auto *fx: weak(track->midi_fx)) {
		fx->reset_state();
		fx->process(events);
	}
	midi_streamer->set_data(events);
	midi_streamer->ignore_end = true;
	synth->reset_state();

}

void TrackRenderer::on_track_add_or_delete_fx() {
	if (!track)
		return;
	if (direct_mode) {
		fx = track->fx;
	}
	for (auto *f: weak(fx))
		f->perf_set_parent(this);
}

void TrackRenderer::on_track_replace_synth() {
	if (!track)
		return;
	synth->perf_set_parent(nullptr);
	if (direct_mode) {
		synth = track->synth;
	} else {
		synth = (Synthesizer*)track->synth->copy();
		synth->set_sample_rate(track->song->sample_rate);
		synth->set_instrument(track->instrument);
	}

	if (track->type == SignalType::MIDI) {
		synth->_plug_in(0, midi_streamer.get(), 0);
	} else if (track->type == SignalType::BEATS) {
		synth->_plug_in(0, beat_midifier.get(), 0);
	}
	synth->perf_set_parent(this);
}

void TrackRenderer::on_track_change_data() {
	if (midi_streamer) {
		fill_midi_streamer();
	}
}

void TrackRenderer::update_layers() {
	for (auto *l: weak(layers))
		l->unsubscribe(this);

	//msg_write("-----TR update  clear");
	layers.clear();
	//msg_write("-----TR update =");
	if (track)
		layers = track->layers;
	//msg_write("-----TR update done");

	for (auto *l: weak(layers)) {
		l->subscribe(this, [=]{ on_track_change_data(); }, l->MESSAGE_CHANGE);
		//l->subscribe(this, [=]{ on_track_delete_layer(); }, l->MESSAGE_DELETE);
	}
}

void TrackRenderer::set_pos(int pos) {
	if (!track)
		return;
	offset = pos;
	if (midi_streamer)
		midi_streamer->set_pos(pos);
	for (auto *f: weak(fx))
		f->reset_state();
	if (track->type == SignalType::BEATS)
		song_renderer->bar_streamer->set_pos(pos);
}

static void copy_direct(TrackLayer *l, const Range &r, AudioBuffer &buf, const Range &cur) {
	Range r1 = r and cur;
	AudioBuffer tbuf;
	tbuf.set_as_ref(buf, r1.start() - cur.start(), r1.length);
	l->read_buffers_fixed(tbuf, r1);
}

static void add_direct(TrackLayer *l, const Range &r, AudioBuffer &buf, const Range &cur) {
	Range r1 = r and cur;
	if (r1.length <= 0)
		return;

	AudioBuffer tbuf;
	tbuf.resize(r1.length);
	l->read_buffers_fixed(tbuf, r1);
	add_samples(l, r1, tbuf);

	buf.add(tbuf, r1.start() - cur.start());
}

static void add_fade_out(TrackLayer *l, const Range &r, AudioBuffer &buf, const Range &cur) {
	Range r1 = r and cur;
	AudioBuffer tbuf;
	tbuf.resize(r1.length);
	l->read_buffers_fixed(tbuf, r1);
	add_samples(l, r1, tbuf);

	// perform (linear) fade
	for (int i=r1.start(); i<r1.end(); i++) {
		float a = (float)(r.end() - i) / (float)r.length;
		for (int c=0; c<buf.channels; c++)
			tbuf.c[c][i - r1.start()] *= a;
	}
	buf.add(tbuf, r1.start() - cur.start());
}

static void add_fade_in(TrackLayer *l, const Range &r, AudioBuffer &buf, const Range &cur) {
	Range r1 = r and cur;
	AudioBuffer tbuf;
	tbuf.resize(r1.length);
	l->read_buffers_fixed(tbuf, r1);
	add_samples(l, r1, tbuf);

	// perform (linear) fade
	for (int i=r1.start(); i<r1.end(); i++) {
		float a = (float)(i - r.start()) / (float)r.length;
		for (int c=0; c<buf.channels; c++)
			tbuf.c[c][i - r1.start()] *= a;
	}
	buf.add(tbuf, r1.start() - cur.start());
}

void TrackRenderer::render_audio_versioned(AudioBuffer &buf) {
	Range cur = Range(offset, buf.length);
	
	for (TrackLayer *l: weak(track->layers)) {
		if (song_renderer and !song_renderer->allowed_layers.contains(l))
			continue;
	
		bool prev_active = true;
		int prev_end = offset;
		
		for (auto &f: l->fades) {
			Range r = f.range();

			// before
			if (prev_active and (prev_end < cur.end()))
				add_direct(l, Range::to(prev_end, r.start()), buf, cur);

			// fade
			if (r.overlaps(cur)) {
				if (f.mode == f.INWARD)
					add_fade_in(l, r, buf, cur);
				else
					add_fade_out(l, r, buf, cur);
			}
			
			prev_active = (f.mode == f.INWARD);
			prev_end = f.range().end();
		}
		
		// after
		if (prev_active and (prev_end < cur.end()))
			add_direct(l, Range::to(prev_end, cur.end()), buf, cur);
	}
}

void TrackRenderer::render_audio_layered(AudioBuffer &buf) {
	Range cur = Range(offset, buf.length);

	// any un-muted layer?
	int i0 = get_first_usable_layer();
	if (i0 < 0) {
		// no -> return silence
		buf.set_zero();
	} else {

		// first (un-muted) layer
		track->layers[i0]->read_buffers_fixed(buf, cur);
		// TODO: allow_ref if no other layers + no fx
		add_samples(track->layers[i0].get(), cur, buf);

		// other layers
		AudioBuffer tbuf;
		for (int i=i0+1;i<track->layers.num;i++) {
			if (song_renderer and !song_renderer->allowed_layers.contains(track->layers[i].get()))
				continue;

			//if (track->layers[i]->muted)
			//	continue;
			track->layers[i]->read_buffers(tbuf, cur, true);
			add_samples(track->layers[i].get(), cur, tbuf);
			buf.add(tbuf, 0);
		}
	}

	// mono?
	buf.channels = track->channels;
}

void TrackRenderer::render_audio(AudioBuffer &buf) {
	if (track->has_version_selection()) {
		render_audio_versioned(buf);
	} else {
		render_audio_layered(buf);
	}
}

void TrackRenderer::render_time(AudioBuffer &buf) {
	synth->port_out[0]->read_audio(buf);
}

void TrackRenderer::render_midi(AudioBuffer &buf) {
	synth->port_out[0]->read_audio(buf);
}

void TrackRenderer::render_group(AudioBuffer &buf) {
	song_renderer->render_send_target(buf, track.get());
}

void TrackRenderer::render_no_fx(AudioBuffer &buf) {
	if (!track)
		return;
	if (track->type == SignalType::AUDIO)
		render_audio(buf);
	else if (track->type == SignalType::BEATS)
		render_time(buf);
	else if (track->type == SignalType::MIDI)
		render_midi(buf);
	else if (track->type == SignalType::GROUP)
		render_group(buf);
	offset += buf.length;
}

void TrackRenderer::apply_fx(AudioBuffer &buf, Array<AudioEffect*> &fx_list) {
	// apply fx
	for (AudioEffect *f: fx_list)
		if (f->enabled) {
			f->perf_start();
			f->process(buf);
			f->perf_end();
		}
}

float get_max_volume(const Array<float> &buf) {
	float peak = 0;
	for (int i=0; i<buf.num; i++)
		peak = max(peak, (float)fabs(buf[i]));
	return peak;
}


void apply_curves(Track *track, int pos) {
	for (Curve *c: weak(track->curves))
		c->apply(pos);
}

void unapply_curves(Track *track) {
	for (Curve *c: weak(track->curves))
		c->unapply();
}

int TrackRenderer::read(AudioBuffer &buf) {
	perf_start();

	if (track->curves.num > 0) {
		for (int d=0; d<buf.length; d+=CURVE_CHUNK) {
			AudioBuffer tbuf;
			tbuf.set_as_ref(buf, d, min(buf.length- d, CURVE_CHUNK));
			read_basic(tbuf);
		}
	} else {
		read_basic(buf);
	}


	peak[0] = max(peak[0], get_max_volume(buf.c[0]));
	peak[1] = max(peak[1], get_max_volume(buf.c[1]));
	perf_end();
	return buf.length;
}

int TrackRenderer::read_basic(AudioBuffer &buf) {
	apply_curves(track.get(), buf.offset);
	render_no_fx(buf);

	auto _fx = fx;
	if (song_renderer and song_renderer->preview_effect)
		_fx.add(song_renderer->preview_effect);
	apply_fx(buf, weak(_fx));

	buf.mix_stereo(track->volume, track->panning);

	unapply_curves(track.get());
	return buf.length;
}
