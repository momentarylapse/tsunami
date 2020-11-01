/*
 * SongRenderer.cpp
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#include "SongRenderer.h"
#include "TrackRenderer.h"
#include "AudioEffect.h"
#include "../Beats/BarStreamer.h"
#include "../Beats/BeatMidifier.h"
#include "../Synth/Synthesizer.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Curve.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../lib/math/math.h"




SongRenderer::SongRenderer(Song *s, bool _direct_mode) {
	module_subtype = "SongRenderer";
	MidiEventBuffer no_midi;
	song = s;
	channels = 2;
	direct_mode = _direct_mode;

	allow_loop = false; // are we allowed to loop (recording...)
	loop = false; // do we want to loop (GUI)
	pos = 0;
	needs_rebuild = true;
	needs_synth_reset = true;
	_previous_pos_delta = 0;
	if (song) {
		build_data();
		set_range(song->range());
		allow_layers(layer_set(song->layers()));
		song->subscribe(this, [=]{ on_song_add_track(); }, song->MESSAGE_ADD_TRACK);
		song->subscribe(this, [=]{ on_song_delete_track(); }, song->MESSAGE_DELETE_TRACK);
		song->subscribe(this, [=]{ on_song_finished_loading(); }, song->MESSAGE_FINISHED_LOADING);
	}
}

SongRenderer::~SongRenderer() {
	if (song) {
		song->unsubscribe(this);
		clear_data();
	}
}

void SongRenderer::__init__(Song *s) {
	new(this) SongRenderer(s);
}

void SongRenderer::__delete__() {
	this->SongRenderer::~SongRenderer();
}

int SongRenderer::get_first_usable_track(Track *target) {
	foreachi(auto *tr, tracks, i)
		if (!tr->track->muted and allowed_tracks.contains(tr->track.get()) and (tr->track->send_target == target))
			return i;
	return -1;
}

void SongRenderer::render_send_target(AudioBuffer &buf, Track* target) {
	// any un-muted track?
	int i0 = get_first_usable_track(target);
	if (i0 < 0) {
		// no -> return silence
		buf.set_zero();
	} else {

		// first (un-muted) track
		tracks[i0]->read(buf);

		// other tracks
		for (int i=i0+1;i<tracks.num;i++){
			Track *t = tracks[i]->track.get();
			if (!allowed_tracks.contains(t))
				continue;
			if (t->muted)
				continue;
			if (t->send_target != target)
				continue;
			AudioBuffer tbuf;
			tbuf.resize(buf.length);
			tracks[i]->read(tbuf);
			buf.add(tbuf, 0, 1);
		}

		//buf.mix_stereo(song->volume);
	}
}

void SongRenderer::render_song_no_fx(AudioBuffer &buf) {
	render_send_target(buf, nullptr);
}

void apply_curves(Song *audio, int pos) {
	for (Curve *c: weak(audio->curves))
		c->apply(pos);
}

void unapply_curves(Song *audio) {
	for (Curve *c: weak(audio->curves))
		c->unapply();
}

void SongRenderer::read_basic(AudioBuffer &buf) {
	range_cur = Range(pos, buf.length);
	channels = buf.channels;

	apply_curves(song, pos);

	// render without fx
	buf.set_zero();
	render_song_no_fx(buf);

	unapply_curves(song);

	pos += buf.length;
}

int SongRenderer::read(AudioBuffer &buf) {
	if (!song)
		return 0;

	std::lock_guard<std::shared_timed_mutex> lck(song->mtx);

	if (needs_rebuild)
		_rebuild();
	if (needs_synth_reset)
		_reset_all_synth();

	int size = min(buf.length, _range.end() - pos);
	if (size <= 0)
		return Port::END_OF_STREAM;


	bar_streamer->bars = song->bars;
	// in case, the metronome track is muted
	bar_streamer->set_pos(pos);

	buf.offset = pos;

	if (song->curves.num >= 0) {
		int chunk = 1024;
		for (int d=0; d<size; d+=chunk) {
			AudioBuffer tbuf;
			tbuf.set_as_ref(buf, d, min(size - d, chunk));
			read_basic(tbuf);
		}
	} else {
		read_basic(buf);
	}

	if ((pos >= _range.end()) and allow_loop and loop) {
		reset_state();
		_previous_pos_delta = _range.length;
		_set_pos(_range.offset);
	}
	return size;
}

void SongRenderer::render(const Range &range, AudioBuffer &buf) {
	channels = buf.channels;
	allow_loop = false;
	set_range(range);
	buf.resize(range.length);
	read(buf);
}

void SongRenderer::change_range(const Range &r) {
	if (r == _range)
		return;
	_range = r;
	needs_rebuild = true;
}

void SongRenderer::set_range(const Range &r) {
	change_range(r);
	set_pos(r.start());
}

void SongRenderer::set_loop(bool l) {
	if (l == loop)
		return;
	loop = l;
	needs_rebuild = true;
}

void SongRenderer::allow_layers(const Set<const TrackLayer*> &_allowed_layers) {
	allowed_layers_requested = _allowed_layers;
	needs_rebuild = true;
}

void SongRenderer::_reset_all_synth() {
	for (auto *tr: tracks)
		tr->synth->reset_state();
	needs_synth_reset = false;
}

void SongRenderer::_rebuild() {

	// reset previously unused tracks
	for (auto *tr: tracks)
		if (!allowed_tracks.contains(tr->track.get())) {
			tr->reset_state();
			tr->set_pos(pos);
		}

	allowed_layers = allowed_layers_requested;

	// which tracks are allowed?
	allowed_tracks.clear();
	for (auto *l: allowed_layers)
		allowed_tracks.add(l->track);

	_set_pos(pos);

	needs_rebuild = false;
}

// REQUIRES LOCK
void SongRenderer::clear_data() {
	tracks.clear();

	allowed_tracks.clear();
	allowed_layers.clear();
}


void SongRenderer::reset_state() {
	if (!song)
		return;
	if (preview_effect)
		preview_effect->reset_state();

	for (auto *t: tracks)
		t->reset_state();
}

void SongRenderer::build_data() {
	bar_streamer = new BarStreamer(song->bars);
	bar_streamer->perf_set_parent(this);

	for (Track *t: weak(song->tracks))
		tracks.add(new TrackRenderer(t, this));

	_set_pos(0);
}

int SongRenderer::get_num_samples() const {
	if (allow_loop and loop)
		return -1;
	return _range.length;
}

void SongRenderer::set_pos(int _pos) {
	pos = _pos;
	needs_rebuild = true;
	needs_synth_reset = true;
}

void SongRenderer::_set_pos(int _pos) {
	pos = _pos;
	for (auto &tr: tracks)
		tr->set_pos(pos);
}

int SongRenderer::get_pos(int delta) const {
	Range r = range();
	int p = pos + delta;
	if (p < r.offset)
		p += _previous_pos_delta;
	return p;//loopi(pos + delta, r.start(), r.end());
}

void SongRenderer::on_song_add_track() {
	update_tracks();
}

void SongRenderer::on_song_delete_track() {
	update_tracks();
}

void SongRenderer::on_song_finished_loading() {
	for (auto *tr: tracks)
		tr->on_track_change_data();
}

void SongRenderer::update_tracks() {
	// new tracks
	for (Track *t: weak(song->tracks)) {
		bool found = false;
		for (auto &tr: tracks)
			if (tr->track.get() == t)
				found = true;
		if (!found)
			tracks.add(new TrackRenderer(t, this));
	}

	foreachi (auto &tr, tracks, ti) {
		bool found = weak(song->tracks).find(tr->track.get()) >= 0;
		if (!found) {
			//msg_write("--------SongRenderer erase...");
			tracks.erase(ti);
		}
	}
}

float SongRenderer::get_peak(const Track *t) {
	for (auto &tr: tracks)
		if (tr->track.get() == t) {
			float r = tr->peak;
			tr->peak = 0;
			return r;
		}
	return 0;
}

void SongRenderer::clear_peaks() {
	for (auto &tr: tracks)
		tr->peak = 0;
}

