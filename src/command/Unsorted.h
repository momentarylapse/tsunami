/*
 * Unsorted.h
 *
 *  Created on: Oct 23, 2022
 *      Author: michi
 */

#ifndef SRC_COMMAND_UNSORTED_H_
#define SRC_COMMAND_UNSORTED_H_

#include "../lib/base/base.h"
#include "../lib/base/set.h"

namespace hui {
	class Window;
}

namespace tsunami {

class Song;
class Track;
class SongSelection;
struct AudioOutPort;
class AudioBuffer;
class Range;
class TrackLayer;
class Progress;
class AudioView;
class AudioSource;
class AudioEffect;
class MidiEffect;
class MidiSource;

namespace BufferInterpolator {
	enum class Method;
}

Array<Track*> selected_tracks_sorted(AudioView *view);

void song_compress_buffers(Song *song, const SongSelection &sel, const string &codec);
void song_make_buffers_movable(Song *song, const SongSelection &sel);

void write_into_buffer(AudioOutPort &out, AudioBuffer &buf, int len, Progress *prog = nullptr);

void song_render_track(Song *song, const Range &range, const base::set<const TrackLayer*> &layers, hui::Window *win);

void song_delete_tracks(Song *song, const Array<const Track*> &tracks);
void song_group_tracks(Song *song, const Array<Track*> &tracks);
void song_ungroup_tracks(Song *song, const Array<Track*> &tracks);



void fx_process_layer(TrackLayer *l, const Range &r, AudioEffect *fx, hui::Window *win);
void source_process_layer(TrackLayer *l, const Range &r, AudioSource *fx, hui::Window *win);

int song_apply_volume(Song *song, float volume, bool maximize, const SongSelection &sel, hui::Window *win);
int song_apply_audio_effect(Song *song, AudioEffect *fx, const SongSelection &sel, hui::Window *win);
int song_apply_audio_source(Song *song, AudioSource *s, const SongSelection &sel, hui::Window *win);
int song_apply_midi_effect(Song *song, MidiEffect *fx, const SongSelection &sel, hui::Window *win);
int song_apply_midi_source(Song *song, MidiSource *s, const SongSelection &sel, hui::Window *win);

int song_audio_scale_pitch_shift(Song *song, int new_size, BufferInterpolator::Method method, float pitch_factor, const SongSelection &sel, hui::Window *win);

void song_delete_shift(Song *song, const SongSelection &sel);

}

#endif /* SRC_COMMAND_UNSORTED_H_ */
