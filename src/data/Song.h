/*
 * Song.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_SONG_H_
#define SRC_DATA_SONG_H_

#include "Data.h"
#include "rhythm/BarCollection.h"
#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/any/any.h"
#include <shared_mutex>

class Data;
class AudioEffect;
class MidiEffect;
class Track;
class TrackLayer;
class Sample;
class Synthesizer;
class SongSelection;
class AudioBuffer;
class BarPattern;
class TrackMarker;
class MidiNoteBuffer;
enum class SampleFormat;
enum class SignalType;

class Tag {
public:
	string key, value;
	bool operator==(const Tag &o) const;
	bool operator!=(const Tag &o) const;
	bool valid() const;
};

class Song : public Data {
public:
	Song(Session *session, int sample_rate);
	virtual ~Song();

	void _cdecl __init__(Session *session, int sample_rate);
	void _cdecl __delete__() override;

	Range _cdecl range();
	Range _cdecl range_no_bars();

	obs::source out_new{this, "new"};
	obs::source out_track_list_changed{this, "track-list-changed"};
	obs::source out_sample_list_changed{this, "sample-list-changed"};
	obs::source out_layer_list_changed{this, "layer-list-changed"};
	//obs::source out_edit_layer{this, "edit-layer"};
	obs::source out_channels_changed{this, "channels-changed"};
	obs::source out_edit_bars{this, "edit-bars"};
	obs::source out_scale_bars{this, "scale-bars"};
	obs::source out_enable_fx{this, "enable-fx"};



	class Error : public Exception {
	public:
		explicit Error(const string &message);
	};


	string _cdecl get_time_str(int t);
	string _cdecl get_time_str_fuzzy(int t, float dt);
	string _cdecl get_time_str_long(int t);

	void _cdecl reset() override;
	bool is_empty();

	Track *_cdecl time_track();
	int _cdecl bar_offset(int index);

	string _cdecl get_tag(const string &key);

	Array<TrackMarker*> get_parts();

	// action
	void _cdecl add_tag(const string &key, const string &value);
	void _cdecl edit_tag(int index, const string &key, const string &value);
	void _cdecl delete_tag(int index);
	void _cdecl change_track_volumes(Track *t_ref, const Array<const Track*> &tracks, float volume);
	void _cdecl set_sample_rate(int sample_rate);
	void _cdecl set_default_format(SampleFormat format);
	void _cdecl set_compression(int compression);
	Track *_cdecl add_track(SignalType type, int index = -1);
	Track *_cdecl add_track_after(SignalType type, Track *insert_after = nullptr);
	void _cdecl delete_track(Track *track);
	Sample *_cdecl create_sample_audio(const string &name, const AudioBuffer &buf);
	Sample *_cdecl create_sample_midi(const string &name, const MidiNoteBuffer &midi);
	void _cdecl add_sample(Sample *s);
	void _cdecl delete_sample(Sample *s);
	void _cdecl edit_sample_name(Sample *s, const string &name);
	void _cdecl sample_replace_buffer(Sample *s, AudioBuffer *buf);
	void _cdecl add_bar(int index, const BarPattern &bar, int mode);
	void _cdecl add_pause(int index, int length, int mode);
	void _cdecl edit_bar(int index, const BarPattern &bar, int mode);
	void _cdecl delete_bar(int index, bool affect_midi);
	void _cdecl delete_time_interval(int index, const Range &range);
	void _cdecl insert_selected_samples(const SongSelection &sel);
	void _cdecl delete_selected_samples(const SongSelection &sel);
	void _cdecl delete_selection(const SongSelection &sel);
	void _cdecl create_samples_from_selection(const SongSelection &sel, bool auto_delete);

	// helper
	Sample* _cdecl get_sample_by_uid(int uid);

// data
	Array<Tag> tags;
	int sample_rate;
	SampleFormat default_format;
	int compression;

	shared_array<AudioEffect> __fx;
	shared_array<Track> tracks;
	shared_array<Sample> samples;
	BarCollection bars;

	Any secret_data;

	Array<TrackLayer*> layers() const;
};



int get_track_index(Track *t);


#endif /* SRC_DATA_SONG_H_ */
