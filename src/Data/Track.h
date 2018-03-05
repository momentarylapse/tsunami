/*
 * Track.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef SRC_DATA_TRACK_H_
#define SRC_DATA_TRACK_H_

#include "Range.h"
#include "../Midi/MidiData.h"
#include "Sample.h"
#include "../Midi/Instrument.h"
#include "../lib/math/rect.h"
#include "../Stuff/Observable.h"
#include "../Audio/AudioBuffer.h"
#include "Song.h"



class AudioBuffer;
class Song;
class Synthesizer;
class Effect;




class TrackLayer
{
public:
	Array<AudioBuffer> buffers;
};

class TrackMarker
{
public:
	Range range;
	string text;
	Array<Effect*> fx;
};

string track_type(int type);

class Track : public Observable<VirtualBase>
{
public:
	Track(int type, Synthesizer *synth);
	virtual ~Track();
	Range _cdecl getRange();

	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_MIDI_EFFECT;
	static const string MESSAGE_DELETE_MIDI_EFFECT;

	void _cdecl invalidateAllPeaks();
	AudioBuffer _cdecl readBuffers(int layer_no, const Range &r);
	void _cdecl readBuffersCol(AudioBuffer &buf, int offset);

	string _cdecl getNiceName();
	int _cdecl get_index();

	// actions
	void _cdecl setName(const string &name);
	void _cdecl setInstrument(const Instrument &instrument);
	void _cdecl setMuted(bool muted);
	void _cdecl setVolume(float volume);
	void _cdecl setPanning(float panning);
	void _cdecl move(int target);
	AudioBuffer _cdecl getBuffers(int layer_no, const Range &r);
	void _cdecl insertMidiData(int offset, const MidiNoteBuffer &midi);
	void _cdecl addEffect(Effect *effect);
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
	void _cdecl addMidiNote(const MidiNote &n);
	void _cdecl addMidiNotes(const MidiNoteBuffer &notes);
	void _cdecl deleteMidiNote(int index);
	void _cdecl setSynthesizer(Synthesizer *synth);
	void _cdecl editSynthesizer(const string &param_old);
	void _cdecl detuneSynthesizer(int pitch, float dpitch, bool all_octaves);
	TrackMarker* _cdecl addMarker(const Range &range, const string &text);
	void _cdecl deleteMarker(int index);
	void _cdecl moveMarker(int index, int pos);
	void _cdecl editMarker(int index, const string &text);


	enum
	{
		TYPE_AUDIO,
		TYPE_TIME,
		TYPE_MIDI
	};

// data
	int type;
	string name;

	Instrument instrument;

	Array<TrackLayer> layers;

	float volume, panning;
	bool muted;

	Array<Effect*> fx;
	Array<SampleRef*> samples;

	// midi track
	MidiNoteBuffer midi;
	Synthesizer *synth;



	Array<TrackMarker*> markers;

	Song *song;
};

#endif /* SRC_DATA_TRACK_H_ */
