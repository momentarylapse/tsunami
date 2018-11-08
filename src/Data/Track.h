/*
 * Track.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_TRACK_H_
#define SRC_DATA_TRACK_H_

#include "Range.h"
#include "Midi/MidiData.h"
#include "Midi/Instrument.h"
#include "../Stuff/Observable.h"



class AudioBuffer;
class Song;
class TrackLayer;
class Synthesizer;
class AudioEffect;
class CrossFade;
class TrackMarker;
enum class SignalType;



class Track : public Observable<VirtualBase>
{
public:
	Track(SignalType type, Synthesizer *synth);
	virtual ~Track();

	Range _cdecl range() const;

	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_MIDI_EFFECT;
	static const string MESSAGE_DELETE_MIDI_EFFECT;
	static const string MESSAGE_REPLACE_SYNTHESIZER;

	void _cdecl invalidate_all_peaks();

	string _cdecl nice_name();
	int _cdecl get_index();

	// actions
	void _cdecl set_name(const string &name);
	void _cdecl set_instrument(const Instrument &instrument);
	void _cdecl set_muted(bool muted);
	void _cdecl set_volume(float volume);
	void _cdecl set_panning(float panning);
	TrackLayer _cdecl *add_layer();
	void _cdecl delete_layer(TrackLayer *layer);
	void _cdecl merge_layers();
	void _cdecl move_layer(int source, int target);
	void _cdecl move(int target);
	void _cdecl set_channels(int channels);
	void _cdecl add_effect(AudioEffect *effect);
	void _cdecl delete_effect(AudioEffect *effect);
	void _cdecl edit_effect(AudioEffect *effect, const string &param_old);
	void _cdecl enable_effect(AudioEffect *effect, bool enabled);
	void _cdecl add_midi_effect(MidiEffect *effect);
	void _cdecl delete_midi_effect(MidiEffect *effect);
	void _cdecl edit_midi_effect(MidiEffect *effect, const string &param_old);
	void _cdecl enable_midi_effect(MidiEffect *effect, bool enabled);
	void _cdecl set_synthesizer(Synthesizer *synth);
	void _cdecl edit_synthesizer(const string &param_old);
	void _cdecl detune_synthesizer(int pitch, float dpitch, bool all_octaves);
	TrackMarker* _cdecl add_marker(const Range &range, const string &text);
	void _cdecl delete_marker(const TrackMarker *marker);
	void _cdecl edit_marker(const TrackMarker *marker, const Range &range, const string &text);


// data
	SignalType type;
	int channels;
	string name;

	Instrument instrument;

	Array<TrackLayer*> layers;

	float volume, panning;
	bool muted;

	Array<AudioEffect*> fx;
	Array<MidiEffect*> midi_fx;

	Synthesizer *synth;

	Array<TrackMarker*> markers;

	Array<CrossFade> fades;
	bool has_version_selection();

	Song *song;
};

#endif /* SRC_DATA_TRACK_H_ */
