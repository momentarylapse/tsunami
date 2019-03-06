/*
 * SongRenderer.h
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_SONGRENDERER_H_
#define SRC_MODULE_AUDIO_SONGRENDERER_H_

#include "../../Data/Range.h"
#include "AudioSource.h"

class MidiEventStreamer;
class BarStreamer;
class BeatMidifier;
class Song;
class Track;
class TrackLayer;
class AudioEffect;
class AudioBuffer;
class Synthesizer;
class Range;
class TrackRenderer;

class SongRenderer : public AudioSource
{
	friend TrackRenderer;
public:
	SongRenderer(Song *s, bool direct_mode = false);
	virtual ~SongRenderer();

	void _cdecl __init__(Song *s);
	void _cdecl __delete__() override;

	// from AudioSource
	int read(AudioBuffer &buf) override;
	void reset() override;
	int get_pos() override;
	void set_pos(int pos) override;

	void _cdecl render(const Range &range, AudioBuffer &buf);
	void _cdecl prepare(const Range &range, bool alllow_loop);
	void _cdecl allow_tracks(const Set<const Track*> &allowed_tracks);
	void _cdecl allow_layers(const Set<const TrackLayer*> &allowed_layers);

	void _cdecl set_range(const Range &r){ _range = r; }
	Range _cdecl range(){ return _range; }

	int _cdecl get_num_samples();

	BeatSource _cdecl *get_beat_source(){ return (BeatSource*)bar_streamer; }

private:
	void read_basic(AudioBuffer &buf);
	void render_song_no_fx(AudioBuffer &buf);

	void on_song_add_track();
	void on_song_delete_track();
	void on_song_finished_loading();
	void update_tracks();

	Song *song;
	Range _range;
	Range range_cur;
	int pos;
	Set<const Track*> allowed_tracks;
	Set<const TrackLayer*> allowed_layers;
	bool direct_mode;

	Array<TrackRenderer*> tracks;


	int get_first_usable_track();

	BarStreamer *bar_streamer;
	BeatMidifier *beat_midifier;

	void clear_data();
	void reset_state();
	void build_data();
	void _set_pos(int pos);

	int channels;

public:
	AudioEffect *preview_effect;
	bool allow_loop;
	bool loop_if_allowed;
};

#endif /* SRC_MODULE_AUDIO_SONGRENDERER_H_ */
