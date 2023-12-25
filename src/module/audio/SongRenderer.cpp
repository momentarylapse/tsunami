/*
 * SongRenderer.cpp
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#include "SongRenderer.h"
#include "TrackRenderer.h"
#include "AudioEffect.h"
#include "../beats/BarStreamer.h"
#include "../beats/BeatMidifier.h"
#include "../synthesizer/Synthesizer.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/math/math.h"




SongRenderer::SongRenderer(Song *s, bool _direct_mode) :
		in_track_list_changed{this, [this] { update_tracks(); }} {
	module_class = "SongRenderer";
	MidiEventBuffer no_midi;
	song = s;
	channels = 2;
	direct_mode = _direct_mode;

	allow_loop = false; // are we allowed to loop (recording...)
	loop = false; // do we want to loop (GUI)
	pos = 0;
	samples_produced = 0;
	needs_rebuild = true;
	needs_synth_reset = true;
	_previous_pos_delta = 0;
	if (song) {
		build_data_once();
		set_range(song->range());
		allow_layers(layer_set(song->layers()));
		song->out_track_list_changed >> in_track_list_changed;
		song->out_finished_loading >> create_sink([this] { on_song_finished_loading(); });
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
		if (allowed_tracks.contains(tr->track.get()) and (tr->track->send_target == target))
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

void SongRenderer::read_basic(AudioBuffer &buf) {
	range_cur = Range(pos, buf.length);
	channels = buf.channels;

	// render without fx
	buf.set_zero();
	render_song_no_fx(buf);

	pos += buf.length;
	samples_produced += buf.length;
}

int SongRenderer::read(AudioBuffer &buf) {
	if (!song)
		return 0;

	std::lock_guard<std::shared_timed_mutex> lck(song->mtx);

	if (needs_rebuild)
		_rebuild();
	if (needs_synth_reset)
		_reset_all_synth();

	buf.offset = pos;

	int size = min(buf.length, _range.end() - pos);
	if (size <= 0)
		return Port::END_OF_STREAM;


	bar_streamer->bars = song->bars;
	// in case, the metronome track is muted
	bar_streamer->set_pos(pos);

	auto r = buf.ref(0, size);
	read_basic(r);

	// loop back?
	if ((pos >= _range.end()) and allow_loop and loop) {
		clear_track_data();
		_previous_pos_delta = _range.length;
		_set_pos(_range.offset, false);
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

void SongRenderer::allow_layers(const base::set<const TrackLayer*> &_allowed_layers) {
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

	needs_rebuild = false;
}

// REQUIRES LOCK
void SongRenderer::clear_data() {
	tracks.clear();

	allowed_tracks.clear();
	allowed_layers.clear();
}

void SongRenderer::clear_track_data() {
	if (!song)
		return;
	if (preview_effect)
		preview_effect->reset_state();

	for (auto *t: tracks)
		t->reset_state();
}


void SongRenderer::reset_state() {
	samples_produced = 0;
	clear_track_data();
}

int64 SongRenderer::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::SAMPLE_COUNT_MODE)
		return (int)SampleCountMode::TRANSLATOR;
	if (cmd == ModuleCommand::GET_SAMPLE_COUNT)
		return map_to_pos(param);
	return COMMAND_NOT_HANDLED;
}

void SongRenderer::build_data_once() {
	bar_streamer = new BarStreamer(song->bars);
	bar_streamer->perf_set_parent(this);

	for (Track *t: weak(song->tracks))
		tracks.add(new TrackRenderer(t, this));

	_set_pos(0, false);
}

int SongRenderer::get_num_samples() const {
	if (allow_loop and loop)
		return -1;
	return _range.length;
}

void SongRenderer::set_pos(int _pos) {
	_set_pos(_pos, true);
}

void SongRenderer::_set_pos(int _pos, bool reset_fx) {
	pos = _pos;
	sample_map.add({samples_produced, pos});
	while (sample_map.num > 16)
		sample_map.erase(0);
	for (auto &tr: tracks)
		tr->set_pos(pos);
	needs_rebuild = true;
	needs_synth_reset = true;
}

int SongRenderer::get_pos() const {
	return pos;
}


int64 SongRenderer::get_samples_produced() const {
	return samples_produced;
}

int SongRenderer::map_to_pos(int64 samples_played) const {
	for (int i=sample_map.num-1; i>=0; i--)
		if (samples_played >= sample_map[i].sample_count)
			return sample_map[i].pos + (samples_played - sample_map[i].sample_count);
	return samples_played;
}

void SongRenderer::on_song_finished_loading() {
	update_tracks();
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

void SongRenderer::get_peak(const Track *t, float p[2]) {
	for (auto &tr: tracks)
		if (tr->track.get() == t) {
			p[0] = tr->peak[0];
			p[1] = tr->peak[1];
			tr->peak[0] = 0;
			tr->peak[1] = 0;
			return;
		}
}

void SongRenderer::clear_peaks() {
	for (auto &tr: tracks)
		tr->peak[0] = tr->peak[1] = 0;
}

