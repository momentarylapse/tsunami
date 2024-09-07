#include "Unsorted.h"
#include "../data/base.h"
#include "../data/Song.h"
#include "../data/SongSelection.h"
#include "../data/Track.h"
#include "../data/TrackLayer.h"
#include "../data/audio/AudioBuffer.h"
#include "../module/audio/SongRenderer.h"
#include "../module/audio/AudioEffect.h"
#include "../module/audio/AudioSource.h"
#include "../module/midi/MidiEffect.h"
#include "../module/midi/MidiSource.h"
#include "../storage/Storage.h"
#include "../view/helper/Progress.h"
#include "../action/song/ActionSongMoveSelection.h"
#include "../processing/audio/BufferPitchShift.h"
#include "../Session.h"
#include "../lib/hui/language.h"

#include <stdio.h>

namespace tsunami {

void song_compress_buffers(Song *song, const SongSelection &sel, const string &codec) {
	for (auto l: song->layers())
		if (sel.has(l))
			for (auto &buf: l->buffers)
				if (buf.range().overlaps(sel.range())) {
					shared<AudioBuffer::Compressed> comp = new AudioBuffer::Compressed;
					comp->codec = codec;
					comp->data = song->session->storage->compress(buf, comp->codec);
					if (comp->data.num > 0) {
						buf.compressed = comp;
						l->out_changed.notify();
					}
				}
}


void song_make_buffers_movable(Song *song, const SongSelection &sel) {
	song->begin_action_group("make movable");
	for (auto l: song->layers())
		if (sel.has(l)) {
			Array<Range> ranges;
			for (auto &buf: l->buffers)
				if (buf.range().overlaps(sel.range()))
					ranges.add(buf.range());
			for (auto &r: ranges) {
				auto s = SongSelection::from_range(song, r).filter({l}).filter(0);
				song->create_samples_from_selection(s, true);
			}
		}
	song->end_action_group();
}


void write_into_buffer(AudioOutPort &out, AudioBuffer &buf, int len, Progress *prog) {
	const int chunk_size = 1 << 12;
	int offset = 0;

	while (offset < len) {
		if (prog)
			prog->set((float) offset / len);

		Range r = Range::to(offset, min(offset + chunk_size, len));

		AudioBuffer tbuf;
		tbuf.set_as_ref(buf, offset, r.length);

		out.read_audio(tbuf);

		offset += chunk_size;
		if (prog)
			if (prog->is_cancelled())
				break;
	}
}


void song_render_track(Song *song, const Range &range, const base::set<const TrackLayer*> &layers, hui::Window *win) {
	SongRenderer renderer(song);
	renderer.set_range(range);
	renderer.allow_layers(layers);

	auto p = ownify(new ProgressCancelable(_(""), win));

	AudioBuffer buf;
	buf.resize(range.length);

	write_into_buffer(renderer.out, buf, range.length, p.get());

	song->begin_action_group(_("render track"));
	Track *t = song->add_track(SignalType::AudioStereo);
	AudioBuffer buf_track;
	auto *a = t->layers[0]->edit_buffers(buf_track, range);
	buf_track.set(buf, 0, 1);

	t->layers[0]->edit_buffers_finish(a);
	song->end_action_group();
}

void song_delete_tracks(Song *song, const Array<const Track*> &tracks) {
	song->begin_action_group(_("delete tracks"));
	for (auto t: tracks) {
		try {
			song->delete_track(const_cast<Track*>(t));
		} catch (Exception &e) {
			song->session->e(e.message());
		}
	}
	song->end_action_group();
}


// tracks needs to be sorted!!!
void song_group_tracks(Song *song, const Array<Track*> &tracks) {
	if (tracks.num == 0)
		return;
	song->begin_action_group(_("group tracks"));
	int first_index = tracks[0]->get_index();
	auto group = song->add_track(SignalType::Group, first_index);
	// add to group
	for (auto t: tracks)
		t->set_send_target(group);
	// move into connected group
	foreachi (auto t, tracks, i)
		t->move(first_index + i + 1);
	song->end_action_group();
}


Track *track_top_group(Track *t) {
	if (t->send_target)
		return track_top_group(t->send_target);
	if (t->type == SignalType::Group)
		return t;
	return nullptr;
}

bool track_is_in_group(Track *t, Track *group) {
	if (t == group)
		return true;
	if (t->send_target == group)
		return true;
	if (t->send_target)
		return track_is_in_group(t->send_target, group);
	return false;
}

Array<Track*> track_group_members(Track *group, bool with_self) {
	Array<Track*> tracks;
	for (auto t: weak(group->song->tracks))
		if ((t != group) or with_self)
			if (track_is_in_group(t, group))
				tracks.add(t);
	return tracks;
}

void song_ungroup_tracks(Song *song, const Array<Track*> &tracks) {
	if (tracks.num == 0)
		return;
	song->begin_action_group(_("ungroup tracks"));
	foreachb (auto t, tracks) {
		auto group = track_top_group(t);
		if (group and (group != t)) {
			auto members = track_group_members(group, true);
			t->set_send_target(nullptr);
			t->move(members.back()->get_index());
		}
	}
	song->end_action_group();
}



void fx_process_layer(TrackLayer *l, const Range &r, AudioEffect *fx, hui::Window *win) {
	auto p = ownify(new Progress(_("applying effect"), win));
	fx->reset_state();

	AudioBuffer buf;
	auto *a = l->edit_buffers(buf, r);

	if (fx->apply_to_whole_buffer) {
		fx->apply_with_wetness(buf);
	} else {

		int chunk_size = 2048;
		int done = 0;
		while (done < r.length) {
			p->set((float) done / (float) r.length);

			auto ref = buf.ref(done, min(done + chunk_size, r.length));
			fx->apply_with_wetness(ref);
			done += chunk_size;
		}
	}

	l->edit_buffers_finish(a);
}

void layer_apply_volume(TrackLayer *l, const Range &r, float f, hui::Window *win) {
	auto p = ownify(new Progress(_("applying volume"), win));

	AudioBuffer buf;
	auto *a = l->edit_buffers(buf, r);

	const int CHUNK_SIZE = 2048 * 16;
	int done = 0;
	while (done < r.length) {
		p->set((float) done / (float) r.length);

		auto ref = buf.ref(done, min(done + CHUNK_SIZE, r.length));
		for (auto& c: ref.c)
			for (int i=0; i<ref.length; i++)
				c[i] *= f;
		done += CHUNK_SIZE;
	}

	l->edit_buffers_finish(a);
}

void source_process_layer(TrackLayer *l, const Range &r, AudioSource *fx, hui::Window *win) {
	auto p = ownify(new Progress(_("applying source"), win));
	fx->reset_state();
	
	AudioBuffer buf;
	auto *a = l->edit_buffers(buf, r);
	buf.set_zero();

	const int CHUNK_SIZE = 2048;
	int done = 0;
	while (done < r.length) {
		p->set((float) done / (float) r.length);

		auto ref = buf.ref(done, min(done + CHUNK_SIZE, r.length));
		fx->read(ref);
		done += CHUNK_SIZE;
	}

	l->edit_buffers_finish(a);
}

float song_max_volume(Song *song, const SongSelection &sel) {
	float max_vol = 0;
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (sel.has(l) and (t->type == SignalType::Audio)) {
				AudioBuffer buf;
				l->read_buffers(buf, sel.range(), true);
				for (auto& c: buf.c)
					for (auto f: c)
						max_vol = max(max_vol, abs(f));
			}
	return max_vol;
}

int song_apply_volume(Song *song, float volume, bool maximize, const SongSelection &sel, hui::Window *win) {
	if (maximize)
		volume = 1.0f / song_max_volume(song, sel);

	int n_layers = 0;
	song->begin_action_group(_("apply audio fx"));
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (sel.has(l) and (t->type == SignalType::Audio)) {
				layer_apply_volume(l, sel.range(), volume, win);
				n_layers ++;
			}
	song->end_action_group();
	return n_layers;
}

int song_apply_audio_effect(Song *song, AudioEffect *fx, const SongSelection &sel, hui::Window *win) {
	int n_layers = 0;
	song->begin_action_group(_("apply audio fx"));
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (sel.has(l) and (t->type == SignalType::Audio)) {
				fx_process_layer(l, sel.range(), fx, win);
				n_layers ++;
			}
	song->end_action_group();
	return n_layers;
}

int song_apply_audio_source(Song *song, AudioSource *s, const SongSelection &sel, hui::Window *win) {
	int n_layers = 0;
	song->begin_action_group(_("audio source"));
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (sel.has(l) and (t->type == SignalType::Audio)) {
				source_process_layer(l, sel.range(), s, win);
				n_layers ++;
			}
	song->end_action_group();
	return n_layers;
}

int song_apply_midi_effect(Song *song, MidiEffect *fx, const SongSelection &sel, hui::Window *win) {
	int n_layers = 0;
	song->begin_action_group(_("apply midi fx"));
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (sel.has(l) and (t->type == SignalType::Midi)) {
				fx->reset_state();
				fx->process_layer(l, sel);
				n_layers ++;
			}
	song->end_action_group();
	return n_layers;
}

int song_apply_midi_source(Song *song, MidiSource *s, const SongSelection &sel, hui::Window *win) {
	int n_layers = 0;
	song->begin_action_group(_("midi source"));
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (sel.has(l) and (t->type == SignalType::Midi)) {
				s->reset_state();
				MidiEventBuffer buf;
				buf.samples = sel.range().length;
				s->read(buf);
				l->insert_midi_data(sel.range().offset, midi_events_to_notes(buf));
				n_layers ++;
			}
	song->end_action_group();
	return n_layers;
}

void song_delete_shift(Song *song, const SongSelection &sel) {
	if (sel.is_empty())
		return;
	song->begin_action_group(_("delete shift"));
	song->delete_selection(sel);
	auto sel_after = SongSelection::from_range(song, Range::to(sel.range().end(), 2000000000)).filter(sel.layers());
	auto a = new ActionSongMoveSelection(song, sel_after, true);
	a->set_param_and_notify(song, -sel.range().length);
	song->execute(a);
	song->end_action_group();
}

void layer_audio_scale_pitch_shift(TrackLayer *l, const Range &r, int new_size, BufferInterpolator::Method scaling_method, float pitch_factor, hui::Window *win) {
	auto p = ownify(new Progress(_("applying scale/pitch"), win));

	auto r_affected = Range(r.offset, max(r.length, new_size));

	AudioBuffer buf_layer;
	auto *a = l->edit_buffers(buf_layer, r_affected);
	AudioBuffer buf_in = buf_layer;
	buf_in.make_own();


	const int CHUNK_SIZE = 2048 * 4;
	int done_in = 0;
	int done_out = 0;
	BufferPitchShift::Operator op;
	op.reset((float)new_size / (float)r.length, scaling_method, pitch_factor);

	while (max(done_in, done_out) < r.length) {
		auto buf_edited = op.process(buf_in.ref(done_in, done_in + CHUNK_SIZE));
		buf_layer.ref(done_out, 10000000).set(buf_edited, 0, 1.0f);
		done_in += CHUNK_SIZE;
		done_out += buf_edited.length;
		p->set((float)max(done_in, done_out) / (float)r.length);
	}

	l->edit_buffers_finish(a);
}

int song_audio_scale_pitch_shift(Song *song, int new_size, BufferInterpolator::Method scaling_method, float pitch_factor, const SongSelection &sel, hui::Window *win) {
	int n_layers = 0;
	song->begin_action_group(_("audio scale/pitch"));
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (sel.has(l) and (t->type == SignalType::Audio)) {
				layer_audio_scale_pitch_shift(l, sel.range(), new_size, scaling_method, pitch_factor, win);
				n_layers ++;
			}
	song->end_action_group();
	return n_layers;
}

}
