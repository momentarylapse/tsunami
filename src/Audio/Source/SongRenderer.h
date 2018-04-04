/*
 * SongRenderer.h
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_SONGRENDERER_H_
#define SRC_AUDIO_SOURCE_SONGRENDERER_H_

#include "AudioSource.h"
#include "../../Data/Range.h"

class MidiPort;
class MidiEventStreamer;
class BarStreamer;
class BeatMidifier;
class Song;
class Track;
class AudioEffect;
class AudioBuffer;
class Range;

class SongRenderer : public AudioSource
{
public:
	SongRenderer(Song *s);
	virtual ~SongRenderer();

	void _cdecl __init__(Song *s);
	virtual void _cdecl __delete__();

	// from AudioSource
	virtual int _cdecl read(AudioBuffer &buf);
	virtual void _cdecl reset();
	virtual int _cdecl get_pos(int delta);

	void _cdecl render(const Range &range, AudioBuffer &buf);
	void _cdecl prepare(const Range &range, bool alllow_loop);
	void _cdecl allow_tracks(const Set<Track*> &allowed_tracks);

	void _cdecl seek(int pos);

	void _cdecl set_range(const Range &r){ _range = r; }
	Range _cdecl range(){ return _range; }

	int _cdecl get_num_samples();

private:
	void read_basic(AudioBuffer &buf, int pos);
	void render_audio_track_no_fx(AudioBuffer &buf, Track *t, int ti);
	void render_time_track_no_fx(AudioBuffer &buf, Track *t, int ti);
	void render_midi_track_no_fx(AudioBuffer &buf, Track *t, int ti);
	void render_track_no_fx(AudioBuffer &buf, Track *t, int ti);
	void apply_fx(AudioBuffer &buf, Track *t, Array<AudioEffect*> &fx_list);
	void render_track_fx(AudioBuffer &buf, Track *t, int ti);
	void render_song_no_fx(AudioBuffer &buf);

	void on_song_change();

	Song *song;
	Range _range;
	Range range_cur;
	int pos;
	Set<Track*> allowed_tracks;

	Array<MidiEventStreamer*> midi_streamer;
	BarStreamer *bar_streamer;
	BeatMidifier *beat_midifier;

	void clear_data();
	void reset_state();
	void reset_track_state(Track *t);
	void build_data();
	void _seek(int pos);

public:
	AudioEffect *preview_effect;
	bool allow_loop;
	bool loop_if_allowed;
};

#endif /* SRC_AUDIO_SOURCE_SONGRENDERER_H_ */
