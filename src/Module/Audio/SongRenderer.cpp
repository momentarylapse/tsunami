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
#include "../../Data/Curve.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../lib/math/math.h"




SongRenderer::SongRenderer(Song *s, bool _direct_mode) {
	module_subtype = "SongRenderer";
	MidiEventBuffer no_midi;
	song = s;
	beat_midifier = nullptr;
	bar_streamer = nullptr;
	channels = 2;
	direct_mode = _direct_mode;

	preview_effect = nullptr;
	allow_loop = false;
	loop_if_allowed = false;
	pos = 0;
	if (song) {
		build_data();
		prepare(song->range(), false);
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
		if (!tr->track->muted and allowed_tracks.contains(tr->track) and (tr->track->send_target == target))
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
			Track *t = tracks[i]->track;
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
	for (Curve *c: audio->curves)
		c->apply(pos);
}

void unapply_curves(Song *audio) {
	for (Curve *c: audio->curves)
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

	if ((pos >= _range.end()) and allow_loop and loop_if_allowed) {
		reset_state();
		_set_pos(_range.offset);
	}
	return size;
}

void SongRenderer::render(const Range &range, AudioBuffer &buf) {
	channels = buf.channels;
	prepare(range, false);
	buf.resize(range.length);
	read(buf);
}

void SongRenderer::allow_tracks(const Set<const Track*> &_allowed_tracks) {
	std::lock_guard<std::shared_timed_mutex> lck(song->mtx);
	// reset previously unused tracks
	for (auto *tr: tracks)
		if (!allowed_tracks.contains(tr->track)) {
			tr->reset_state();
			tr->set_pos(pos);
		}
	allowed_tracks = _allowed_tracks;
	//_seek(pos);
}

void SongRenderer::allow_layers(const Set<const TrackLayer*> &_allowed_layers) {
	std::lock_guard<std::shared_timed_mutex> lck(song->mtx);
	allowed_layers = _allowed_layers;
	_set_pos(pos);
}

void SongRenderer::clear_data() {
	for (auto *tr: tracks)
		delete tr;
	tracks.clear();

	if (beat_midifier) {
		delete beat_midifier;
		beat_midifier = nullptr;
	}

	if (bar_streamer) {
		delete bar_streamer;
		bar_streamer = nullptr;
	}

	allowed_tracks.clear();
	allowed_layers.clear();
}

void SongRenderer::prepare(const Range &__range, bool _allow_loop) {
	std::lock_guard<std::shared_timed_mutex> lck(song->mtx);
	//clear_data();
	_range = __range;
	allow_loop = _allow_loop;

	for (Track* t: song->tracks)
		allowed_tracks.add(t);
	for (TrackLayer* l: song->layers())
		allowed_layers.add(l);

	reset_state();
	_set_pos(_range.offset);
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
	beat_midifier = new BeatMidifier;
	beat_midifier->_plug_in(0, bar_streamer, 0);
	beat_midifier->perf_set_parent(this);

	for (Track *t: song->tracks)
		tracks.add(new TrackRenderer(t, this));

	set_pos(0);
}

int SongRenderer::get_num_samples() {
	if (allow_loop and loop_if_allowed)
		return -1;
	return _range.length;
}

void SongRenderer::set_pos(int _pos) {
	std::lock_guard<std::shared_timed_mutex> lck(song->mtx);
	reset_state();
	_set_pos(_pos);
}

void SongRenderer::_set_pos(int _pos) {
	pos = _pos;
	for (auto &tr: tracks)
		tr->set_pos(pos);
}

int SongRenderer::get_pos() {
	int delta = 0;
	Range r = range();
	return loopi(pos + delta, r.start(), r.end());
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
	for (Track *t: song->tracks) {
		bool found = false;
		for (auto &tr: tracks)
			if (tr->track == t)
				found = true;
		if (!found)
			tracks.add(new TrackRenderer(t, this));
	}

	foreachi (auto *tr, tracks, ti) {
		bool found = false;
		for (Track *t: song->tracks)
			if (t == tr->track)
				found = true;
		if (!found) {
			delete tr;
			tracks.erase(ti);
		}
	}
}

float SongRenderer::get_peak(Track *t) {
	for (auto *tr: tracks)
		if (tr->track == t) {
			float r = tr->peak;
			tr->peak = 0;
			return r;
		}
	return 0;
}

void SongRenderer::clear_peaks() {
	for (auto *tr: tracks)
		tr->peak = 0;
}

