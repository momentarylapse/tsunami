/*
 * Track.h
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#ifndef TRACK_H_
#define TRACK_H_

#include "Range.h"
#include "BufferBox.h"
#include "../Midi/MidiData.h"
#include "Sample.h"
#include "../Midi/Instrument.h"
#include "../lib/math/rect.h"
#include "../Stuff/Observable.h"
#include "Song.h"


#define NUM_PEAK_LEVELS		24
#define PEAK_FACTOR			2
#define MIN_MAX_FACTOR		10

#define DEFAULT_SAMPLE_RATE		44100

class BufferBox;
class Song;
class Synthesizer;
class Effect;


class TrackLevel
{
public:
	Array<BufferBox> buffers;
};

class TrackMarker
{
public:
	int pos;
	string text;
};

string track_type(int type);

class Track : public Observable
{
public:
	Track();
	virtual ~Track();
	Range _cdecl getRange();
	Range _cdecl getRangeUnsafe();

	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_MIDI_EFFECT;
	static const string MESSAGE_DELETE_MIDI_EFFECT;

	void _cdecl reset();
	void _cdecl updatePeaks();
	void _cdecl invalidateAllPeaks();
	BufferBox _cdecl readBuffers(int level_no, const Range &r);
	BufferBox _cdecl readBuffersCol(const Range &r);

	string _cdecl getNiceName();
	int _cdecl get_index();

	// actions
	void _cdecl setName(const string &name);
	void _cdecl setInstrument(const Instrument &instrument);
	void _cdecl setMuted(bool muted);
	void _cdecl setVolume(float volume);
	void _cdecl setPanning(float panning);
	BufferBox _cdecl getBuffers(int level_no, const Range &r);
	void _cdecl insertMidiData(int offset, const MidiData &midi);
	void _cdecl addEffect(Effect *effect);
	void _cdecl deleteEffect(int index);
	void _cdecl editEffect(int index, const string &param_old);
	void _cdecl enableEffect(int index, bool enabled);
	void _cdecl addMidiEffect(MidiEffect *effect);
	void _cdecl deleteMidiEffect(int index);
	void _cdecl editMidiEffect(int index, const string &param_old);
	void _cdecl enableMidiEffect(int index, bool enabled);
	SampleRef *_cdecl addSample(int pos, int index);
	void _cdecl deleteSample(int index);
	void _cdecl editSample(int index, float volume, bool mute, int rep_num, int rep_delay);
	void _cdecl addMidiNote(const MidiNote &n);
	void _cdecl addMidiNotes(const MidiData &notes);
	void _cdecl deleteMidiNote(int index);
	void _cdecl setSynthesizer(Synthesizer *synth);
	void _cdecl editSynthesizer(const string &param_old);
	void _cdecl detuneSynthesizer(int pitch, float dpitch, bool all_octaves);
	void _cdecl addMarker(int pos, const string &text);
	void _cdecl deleteMarker(int id);
	void _cdecl moveMarker(int id, int pos);


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

	Array<TrackLevel> levels;

	float volume, panning;
	bool muted;

	Array<Effect*> fx;
	Array<SampleRef*> samples;

	// midi track
	MidiData midi;
	Synthesizer *synth;

	Array<TrackMarker> markers;

	Song *song;
};

#endif /* TRACK_H_ */
