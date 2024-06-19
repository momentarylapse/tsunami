/*
 * SongRenderer.h
 *
 *  Created on: 17.08.2015
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_SONGRENDERER_H_
#define SRC_MODULE_AUDIO_SONGRENDERER_H_

#include "../../data/Range.h"
#include "AudioSource.h"
#include "../../lib/base/set.h"

namespace tsunami {

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

	obs::sink in_track_list_changed;

	// from Module
	void reset_state() override ;
	base::optional<int64> command(ModuleCommand cmd, int64 param) override;

	// from AudioSource
	int read(AudioBuffer &buf) override;

	int get_pos() const;
	void set_pos(int pos);
	int64 get_samples_produced() const;
	int map_to_pos(int64 samples_played) const;

	void _cdecl render(const Range &range, AudioBuffer &buf);
	void _cdecl allow_layers(const base::set<const TrackLayer*> &allowed_layers);

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

	void on_song_finished_loading();
	void update_tracks();
	void _rebuild();
	void _reset_all_synth();
	void clear_track_data();

	Song *song;
	Range _range;
	Range range_cur;
	int pos;
	int64 samples_produced;
	int _previous_pos_delta;
	base::set<const Track*> allowed_tracks;
	base::set<const TrackLayer*> allowed_layers;
	base::set<const TrackLayer*> allowed_layers_requested;
	bool direct_mode;
	bool needs_rebuild;
	bool needs_synth_reset;

	owned_array<TrackRenderer> tracks;


	int get_first_usable_track(Track *target);

	shared<BarStreamer> bar_streamer;

	void clear_data();
	void build_data_once();
	void _set_pos(int pos, bool reset_fx);

	int channels;

	struct SampleMappingOffset {
		int64 sample_count, pos;
	};
	Array<SampleMappingOffset> sample_map;

public:
	shared<AudioEffect> preview_effect;
	bool allow_loop;
	bool loop;

	void get_peak(const Track *track, float p[2]);
	void clear_peaks();
};

}

#endif /* SRC_MODULE_AUDIO_SONGRENDERER_H_ */
