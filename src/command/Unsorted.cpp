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
#include "../Session.h"


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
						l->notify();
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


void write_into_buffer(Port *out, AudioBuffer &buf, int len, Progress *prog) {
	const int chunk_size = 1 << 12;
	int offset = 0;

	while (offset < len) {
		if (prog)
			prog->set((float) offset / len);

		Range r = Range::to(offset, min(offset + chunk_size, len));

		AudioBuffer tbuf;
		tbuf.set_as_ref(buf, offset, r.length);

		out->read_audio(tbuf);

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

	write_into_buffer(renderer.port_out[0], buf, range.length, p.get());

	song->begin_action_group(_("render track"));
	Track *t = song->add_track(SignalType::AUDIO_STEREO);
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
	auto group = song->add_track(SignalType::GROUP, first_index);
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
	if (t->type == SignalType::GROUP)
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
		fx->process(buf);
	} else {

		int chunk_size = 2048;
		int done = 0;
		while (done < r.length) {
			p->set((float) done / (float) r.length);

			auto ref = buf.ref(done, min(done + chunk_size, r.length));
			fx->process(ref);
			done += chunk_size;
		}
	}

	l->edit_buffers_finish(a);
}

void source_process_layer(TrackLayer *l, const Range &r, AudioSource *fx, hui::Window *win) {
	auto p = ownify(new Progress(_("applying source"), win));
	fx->reset_state();
	
	AudioBuffer buf;
	auto *a = l->edit_buffers(buf, r);
	buf.set_zero();

	int chunk_size = 2048;
	int done = 0;
	while (done < r.length) {
		p->set((float) done / (float) r.length);

		auto ref = buf.ref(done, min(done + chunk_size, r.length));
		fx->read(ref);
		done += chunk_size;
	}

	l->edit_buffers_finish(a);
}

int song_apply_audio_effect(Song *song, AudioEffect *fx, const SongSelection &sel, hui::Window *win) {
	int n_layers = 0;
	song->begin_action_group(_("apply audio fx"));
	for (Track *t: weak(song->tracks))
		for (auto *l: weak(t->layers))
			if (sel.has(l) and (t->type == SignalType::AUDIO)) {
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
			if (sel.has(l) and (t->type == SignalType::AUDIO)) {
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
			if (sel.has(l) and (t->type == SignalType::MIDI)) {
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
			if (sel.has(l) and (t->type == SignalType::MIDI)) {
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