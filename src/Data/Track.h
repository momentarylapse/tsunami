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
#include "Sample.h"
#include "Midi/Instrument.h"
#include "../lib/math/rect.h"
#include "../Stuff/Observable.h"
#include "Audio/AudioBuffer.h"
#include "Song.h"



class AudioBuffer;
class Song;
class Synthesizer;
class AudioEffect;




class TrackLayer : public Observable<VirtualBase>
{
public:
	TrackLayer(){}
	TrackLayer(Track *track, bool is_main);
	~TrackLayer();

	Range range(int keep_notes = 0) const;
	AudioBuffer _cdecl readBuffers(const Range &r);

	// actions
	AudioBuffer _cdecl getBuffers(const Range &r);

	Array<AudioBuffer> buffers;

	//MidiNoteBuffer midi;

	Track *track;
	int type;
	bool is_main;
};

class TrackMarker
{
public:
	Range range;
	string text;
	Array<AudioEffect*> fx;
};

string track_type(int type);

class Track : public Observable<VirtualBase>
{
public:
	Track(int type, Synthesizer *synth);
	virtual ~Track();

	Range _cdecl range() const;

	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_MIDI_EFFECT;
	static const string MESSAGE_DELETE_MIDI_EFFECT;

	void _cdecl invalidateAllPeaks();
	void _cdecl readBuffersCol(AudioBuffer &buf, int offset);

	string _cdecl getNiceName();
	int _cdecl get_index();

	// actions
	void _cdecl setName(const string &name);
	void _cdecl setInstrument(const Instrument &instrument);
	void _cdecl setMuted(bool muted);
	void _cdecl setVolume(float volume);
	void _cdecl setPanning(float panning);
	TrackLayer _cdecl *addLayer(bool is_main);
	void _cdecl deleteLayer(TrackLayer *layer);
	void _cdecl mergeLayers(int source, int target);
	void _cdecl moveLayer(int source, int target);
	void _cdecl move(int target);
	void _cdecl addEffect(AudioEffect *effect);
	void _cdecl deleteEffect(int index);
	void _cdecl editEffect(int index, const string &param_old);
	void _cdecl enableEffect(int index, bool enabled);
	void _cdecl addMidiEffect(MidiEffect *effect);
	void _cdecl deleteMidiEffect(int index);
	void _cdecl editMidiEffect(int index, const string &param_old);
	void _cdecl enableMidiEffect(int index, bool enabled);
	SampleRef *_cdecl addSampleRef(int pos, Sample* sample);
	void _cdecl deleteSampleRef(SampleRef *ref);
	void _cdecl editSampleRef(SampleRef *ref, float volume, bool mute);
	void _cdecl setSynthesizer(Synthesizer *synth);
	void _cdecl editSynthesizer(const string &param_old);
	void _cdecl detuneSynthesizer(int pitch, float dpitch, bool all_octaves);
	TrackMarker* _cdecl addMarker(const Range &range, const string &text);
	void _cdecl deleteMarker(const TrackMarker *marker);
	void _cdecl editMarker(const TrackMarker *marker, const Range &range, const string &text);


	void _cdecl insertMidiData(int offset, const MidiNoteBuffer &midi);
	void _cdecl addMidiNote(MidiNote *n);
	void _cdecl addMidiNotes(const MidiNoteBuffer &notes);
	void _cdecl deleteMidiNote(const MidiNote *note);


	enum Type{
		AUDIO,
		TIME,
		MIDI
	};

// data
	int type;
	string name;

	Instrument instrument;

	Array<TrackLayer*> layers;
	TrackLayer *prefered_layer;

	float volume, panning;
	bool muted;

	Array<AudioEffect*> fx;
	Array<MidiEffect*> midi_fx;
	Array<SampleRef*> samples;

	MidiNoteBuffer midi;

	Synthesizer *synth;

	Array<TrackMarker*> markers;

	Song *song;
};

#endif /* SRC_DATA_TRACK_H_ */
