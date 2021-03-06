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
#include "../../lib/base/set.h"

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

class SongRenderer : public AudioSource {
	friend TrackRenderer;
public:
	SongRenderer(Song *s, bool direct_mode = false);
	virtual ~SongRenderer();

	void _cdecl __init__(Song *s);
	void _cdecl __delete__() override;

	// from Module
	void reset_state() override ;

	// from AudioSource
	int read(AudioBuffer &buf) override;

	int get_pos(int delta) const;
	void set_pos(int pos);

	void _cdecl render(const Range &range, AudioBuffer &buf);
	void _cdecl allow_layers(const Set<const TrackLayer*> &allowed_layers);

	void _cdecl set_range(const Range &r);
	void _cdecl change_range(const Range &r);
	void _cdecl set_loop(bool loop);
	Range _cdecl range() const { return _range; }

	int _cdecl get_num_samples() const;

	BeatSource _cdecl *get_beat_source() { return (BeatSource*)bar_streamer.get(); }

private:
	void read_basic(AudioBuffer &buf);
	void render_send_target(AudioBuffer &buf, Track *target);
	void render_song_no_fx(AudioBuffer &buf);

	void on_song_add_track();
	void on_song_delete_track();
	void on_song_finished_loading();
	void update_tracks();
	void _rebuild();
	void _reset_all_synth();

	Song *song;
	Range _range;
	Range range_cur;
	int pos;
	int _previous_pos_delta;
	Set<const Track*> allowed_tracks;
	Set<const TrackLayer*> allowed_layers;
	Set<const TrackLayer*> allowed_layers_requested;
	bool direct_mode;
	bool needs_rebuild;
	bool needs_synth_reset;

	owned_array<TrackRenderer> tracks;


	int get_first_usable_track(Track *target);

	shared<BarStreamer> bar_streamer;

	void clear_data();
	void build_data();
	void _set_pos(int pos);

	int channels;

public:
	shared<AudioEffect> preview_effect;
	bool allow_loop;
	bool loop;

	void get_peak(const Track *track, float p[2]);
	void clear_peaks();
};

#endif /* SRC_MODULE_AUDIO_SONGRENDERER_H_ */
