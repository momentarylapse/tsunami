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


// preserve: rbp, rbx, r12-r15

#define REGISTER_PRE \
long long temp_rbx; \
long long temp_rbp; \
long long temp_r12; \
long long temp_r13; \
long long temp_r14; \
long long temp_r15; \
asm volatile("movq %%rbx, %0\n\t" \
		"movq %%rbp, %1\n\t" \
		"movq %%r12, %2\n\t" \
		"movq %%r13, %3\n\t" \
		"movq %%r14, %4\n\t" \
		"movq %%r15, %5\n\t" \
	: "=r" (temp_rbx), "=r" (temp_rbp), "=r" (temp_r12), "=r" (temp_r13), "=r" (temp_r14), "=r" (temp_r15) : : );


#define REGISTER_POST \
long long temp2_rbx; \
long long temp2_rbp; \
long long temp2_r12; \
long long temp2_r13; \
long long temp2_r14; \
long long temp2_r15; \
asm volatile("movq %%rbx, %0\n\t" \
		"movq %%rbp, %1\n\t" \
		"movq %%r12, %2\n\t" \
		"movq %%r13, %3\n\t" \
		"movq %%r14, %4\n\t" \
		"movq %%r15, %5\n\t" \
	: "=r" (temp2_rbx), "=r" (temp2_rbp), "=r" (temp2_r12), "=r" (temp2_r13), "=r" (temp2_r14), "=r" (temp2_r15) : : ); \
if (temp_rbx != temp2_rbx) \
	msg_error("rbx"); \
if (temp_rbp != temp2_rbp) \
	msg_error("rbp"); \
if (temp_r12 != temp2_r12) \
	msg_error("r12"); \
if (temp_r13 != temp2_r13) \
	msg_error("r13"); \
if (temp_r14 != temp2_r14) \
	msg_error("r14"); \
if (temp_r15 != temp2_r15) \
	msg_error("r15");




SongRenderer::SongRenderer(Song *s, bool _direct_mode)
{
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
	if (song){
		build_data();
		prepare(song->range(), false);
		song->subscribe(this, [&]{ on_song_add_track(); }, song->MESSAGE_ADD_TRACK);
		song->subscribe(this, [&]{ on_song_delete_track(); }, song->MESSAGE_DELETE_TRACK);
		song->subscribe(this, [&]{ on_song_finished_loading(); }, song->MESSAGE_FINISHED_LOADING);
	}
}

SongRenderer::~SongRenderer()
{
	if (song){
		song->unsubscribe(this);
		clear_data();
	}
}

void SongRenderer::__init__(Song *s)
{
	new(this) SongRenderer(s);
}

void SongRenderer::__delete__()
{
	this->SongRenderer::~SongRenderer();
}

int SongRenderer::get_first_usable_track()
{
	foreachi(auto *tr, tracks, i)
		if (!tr->track->muted and allowed_tracks.contains(tr->track))
			return i;
	return -1;
}

void SongRenderer::render_song_no_fx(AudioBuffer &buf)
{
	// any un-muted track?
	int i0 = get_first_usable_track();
	if (i0 < 0){
		// no -> return silence
		buf.scale(0);
	}else{

		// first (un-muted) track
		tracks[i0]->render_fx(buf);
		buf.scale(tracks[i0]->track->volume, tracks[i0]->track->panning);

		// other tracks
		for (int i=i0+1;i<tracks.num;i++){
			Track *t = tracks[i]->track;
			if (!allowed_tracks.contains(t))
				continue;
			if (t->muted)
				continue;
			AudioBuffer tbuf;
			tbuf.resize(buf.length);
			tracks[i]->render_fx(tbuf);
			buf.add(tbuf, 0, t->volume, t->panning);
		}

		buf.scale(song->volume);
	}
}

void apply_curves(Song *audio, int pos)
{
	for (Curve *c: audio->curves)
		c->apply(pos);
}

void unapply_curves(Song *audio)
{
	for (Curve *c: audio->curves)
		c->unapply();
}

void SongRenderer::read_basic(AudioBuffer &buf)
{
	range_cur = Range(pos, buf.length);
	channels = buf.channels;

	apply_curves(song, pos);

	// render without fx
	buf.scale(0);
	render_song_no_fx(buf);

	// apply global fx
	if (song->fx.num > 0)
		TrackRenderer::apply_fx(buf, song->fx);

	unapply_curves(song);

	pos += buf.length;
}

int SongRenderer::read(AudioBuffer &buf)
{
	std::lock_guard<std::shared_timed_mutex> lck(song->mtx);

	int size = min(buf.length, _range.end() - pos);
	if (size <= 0)
		return AudioPort::END_OF_STREAM;


	bar_streamer->bars = song->bars;
	// in case, the metronome track is muted
	bar_streamer->seek(pos);

	buf.offset = pos;

	if (song->curves.num >= 0){
		int chunk = 1024;
		for (int d=0; d<size; d+=chunk){
			AudioBuffer tbuf;
			tbuf.set_as_ref(buf, d, min(size - d, chunk));
			read_basic(tbuf);
		}
	}else
		read_basic(buf);

	if ((pos >= _range.end()) and allow_loop and loop_if_allowed)
		seek(_range.offset);
	return size;
}

void SongRenderer::render(const Range &range, AudioBuffer &buf)
{
	channels = buf.channels;
	prepare(range, false);
	buf.resize(range.length);
	read(buf);
}

void SongRenderer::allow_tracks(const Set<Track*> &_allowed_tracks)
{
	// reset previously unused tracks
	for (auto *tr: tracks)
		if (!allowed_tracks.contains(tr->track)){
			tr->reset_state();
			tr->seek(pos);
		}
	allowed_tracks = _allowed_tracks;
	//_seek(pos);
}

void SongRenderer::allow_layers(const Set<TrackLayer*> &_allowed_layers)
{
	allowed_layers = _allowed_layers;
	_seek(pos);
}

void SongRenderer::clear_data()
{
	for (auto *tr: tracks)
		delete tr;
	tracks.clear();

	if (beat_midifier){
		delete beat_midifier;
		beat_midifier = nullptr;
	}

	if (bar_streamer){
		delete bar_streamer;
		bar_streamer = nullptr;
	}

	allowed_tracks.clear();
	allowed_layers.clear();
}

void SongRenderer::prepare(const Range &__range, bool _allow_loop)
{
	std::lock_guard<std::shared_timed_mutex> lck(song->mtx);
	//clear_data();
	_range = __range;
	allow_loop = _allow_loop;
	_seek(_range.offset);

	for (Track* t: song->tracks)
		allowed_tracks.add(t);
	for (TrackLayer* l: song->layers())
		allowed_layers.add(l);

	reset_state();
	//build_data();
}

void SongRenderer::reset_state()
{
	if (!song)
		return;
	for (AudioEffect *fx: song->fx)
		fx->reset_state();
	if (preview_effect)
		preview_effect->reset_state();

	for (auto *t: tracks)
		t->reset_state();
}

void SongRenderer::build_data()
{
	bar_streamer = new BarStreamer(song->bars);
	beat_midifier = new BeatMidifier;
	beat_midifier->plug(0, bar_streamer, 0);

	for (Track *t: song->tracks)
		tracks.add(new TrackRenderer(t, this));

	seek(0);
}

void SongRenderer::reset()
{
	reset_state();
}

int SongRenderer::get_num_samples()
{
	if (allow_loop and loop_if_allowed)
		return -1;
	return _range.length;
}

void SongRenderer::seek(int _pos)
{
	reset_state();
	_seek(_pos);
}

void SongRenderer::_seek(int _pos)
{
	pos = _pos;
	for (auto &tr: tracks)
		tr->seek(pos);
}

int SongRenderer::get_pos(int delta)
{
	Range r = range();
	return loopi(pos + delta, r.start(), r.end());
}

void SongRenderer::on_song_add_track()
{
	update_tracks();
}

void SongRenderer::on_song_delete_track()
{
	update_tracks();
}

void SongRenderer::on_song_finished_loading()
{
	for (auto *tr: tracks)
		tr->on_track_change_data();
}

void SongRenderer::update_tracks()
{
	// new tracks
	for (Track *t: song->tracks){
		bool found = false;
		for (auto &tr: tracks)
			if (tr->track == t)
				found = true;
		if (!found)
			tracks.add(new TrackRenderer(t, this));
	}

	foreachi (TrackRenderer *tr, tracks, ti){
		bool found = false;
		for (Track *t: song->tracks)
			if (t == tr->track)
				found = true;
		if (!found){
			delete tr;
			tracks.erase(ti);
		}
	}
}

