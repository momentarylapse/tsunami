/*
 * Track.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_TRACK_H_
#define SRC_DATA_TRACK_H_

#include "Range.h"
#include "midi/MidiData.h"
#include "midi/Instrument.h"
#include "../lib/base/pointer.h"
#include "../lib/pattern/Observable.h"



class AudioBuffer;
class Song;
class TrackLayer;
class Synthesizer;
class AudioEffect;
class CrossFadeOld;
class TrackMarker;
class Temperament;
class Curve;
class CurveTarget;
enum class CurveType;
enum class SignalType;



class Track : public Sharable<Observable<VirtualBase>> {
public:
	Track(Song *song, SignalType type, Synthesizer *synth);
	virtual ~Track();

	Range _cdecl range() const;

	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_MIDI_EFFECT;
	static const string MESSAGE_DELETE_MIDI_EFFECT;
	static const string MESSAGE_REPLACE_SYNTHESIZER;
	static const string MESSAGE_ADD_CURVE;
	static const string MESSAGE_DELETE_CURVE;
	static const string MESSAGE_EDIT_CURVE;

	void _cdecl invalidate_all_peaks();

	string _cdecl nice_name() const;
	int _cdecl get_index() const;

	// actions
	void _cdecl set_name(const string &name);
	void _cdecl set_instrument(const Instrument &instrument);
	void _cdecl set_muted(bool muted);
	void _cdecl set_volume(float volume);
	void _cdecl set_panning(float panning);
	void _cdecl set_send_target(Track *t);
	TrackLayer _cdecl *add_layer();
	void _cdecl delete_layer(TrackLayer *layer);
	void _cdecl merge_layers();
	void _cdecl move_layer(int source, int target);
	void _cdecl move(int target);
	void _cdecl set_channels(int channels);
	void _cdecl add_effect(shared<AudioEffect> effect);
	void _cdecl delete_effect(AudioEffect *effect);
	void _cdecl edit_effect(AudioEffect *effect);
	void _cdecl enable_effect(AudioEffect *effect, bool enabled);
	void _cdecl move_effect(int source, int target);
	void _cdecl add_midi_effect(shared<MidiEffect> effect);
	void _cdecl delete_midi_effect(MidiEffect *effect);
	void _cdecl edit_midi_effect(MidiEffect *effect);
	void _cdecl enable_midi_effect(MidiEffect *effect, bool enabled);
	void _cdecl move_midi_effect(int source, int target);
	void _cdecl set_synthesizer(shared<Synthesizer> synth);
	void _cdecl edit_synthesizer();
	void _cdecl detune_synthesizer(const Temperament &temperament);
	void _cdecl mark_dominant(const Array<const TrackLayer*> &layers, const Range &range);
	Curve* _cdecl add_curve(const string &name, CurveTarget &target);
	void _cdecl delete_curve(Curve *curve);
	void _cdecl edit_curve(Curve *curve, const string &name, float min, float max, CurveType type);
	//void _cdecl curve_set_targets(Curve *curve, CurveTarget &target);
	void _cdecl curve_add_point(Curve *curve, int pos, float value);
	void _cdecl curve_delete_point(Curve *curve, int index);
	void _cdecl curve_edit_point(Curve *curve, int index, int pos, float value);


// data
	SignalType type;
	int channels;
	string name;

	Instrument instrument;

	shared_array<TrackLayer> layers;

	float volume, panning;
	bool muted;

	Track *send_target;

	shared_array<AudioEffect> fx;
	shared_array<MidiEffect> midi_fx;
	void _register_fx(AudioEffect *fx);
	void _register_midi_fx(MidiEffect *fx);

	shared<Synthesizer> synth;
	void _register_synth(Synthesizer *s);

	shared_array<Curve> curves;

	shared_array<TrackMarker> _markers_old;

	Array<CrossFadeOld> _fades_old;
	bool has_version_selection() const;

	Song *song;
};

#endif /* SRC_DATA_TRACK_H_ */
