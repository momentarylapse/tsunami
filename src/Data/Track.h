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
enum class SignalType;



class TrackMarker
{
public:
	Range range;
	string text;
	Array<AudioEffect*> fx;
};

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

	void _cdecl invalidateAllPeaks();

	string _cdecl getNiceName();
	int _cdecl get_index();

	// actions
	void _cdecl setName(const string &name);
	void _cdecl setInstrument(const Instrument &instrument);
	void _cdecl setMuted(bool muted);
	void _cdecl setVolume(float volume);
	void _cdecl setPanning(float panning);
	TrackLayer _cdecl *addLayer();
	void _cdecl deleteLayer(TrackLayer *layer);
	void _cdecl mergeLayers();
	void _cdecl moveLayer(int source, int target);
	void _cdecl move(int target);
	void _cdecl setChannels(int channels);
	void _cdecl addEffect(AudioEffect *effect);
	void _cdecl deleteEffect(AudioEffect *effect);
	void _cdecl editEffect(AudioEffect *effect, const string &param_old);
	void _cdecl enableEffect(AudioEffect *effect, bool enabled);
	void _cdecl addMidiEffect(MidiEffect *effect);
	void _cdecl deleteMidiEffect(MidiEffect *effect);
	void _cdecl editMidiEffect(MidiEffect *effect, const string &param_old);
	void _cdecl enableMidiEffect(MidiEffect *effect, bool enabled);
	void _cdecl setSynthesizer(Synthesizer *synth);
	void _cdecl editSynthesizer(const string &param_old);
	void _cdecl detuneSynthesizer(int pitch, float dpitch, bool all_octaves);
	TrackMarker* _cdecl addMarker(const Range &range, const string &text);
	void _cdecl deleteMarker(const TrackMarker *marker);
	void _cdecl editMarker(const TrackMarker *marker, const Range &range, const string &text);


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

	struct Fade
	{
		int position;
		int target;
		int samples;
		Range range();
	};
	Array<Fade> fades;
	bool has_version_selection();

	Song *song;
};

#endif /* SRC_DATA_TRACK_H_ */
