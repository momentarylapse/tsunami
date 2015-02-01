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
#include "MidiData.h"
#include "Rhythm.h"
#include "Sample.h"
#include "AudioFile.h"
#include "../lib/math/rect.h"
#include "../Stuff/Observable.h"


#define NUM_PEAK_LEVELS		24
#define PEAK_FACTOR			2
#define MIN_MAX_FACTOR		10

#define DEFAULT_SAMPLE_RATE		44100

class BufferBox;
class AudioFile;
class Synthesizer;
class Effect;


class TrackLevel
{
public:
	Array<BufferBox> buffer;
};

string track_type(int type);

class Track : public Observable
{
public:
	Track();
	virtual ~Track();
	Range _cdecl getRange();
	Range getRangeUnsafe();

	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_MIDI_EFFECT;
	static const string MESSAGE_DELETE_MIDI_EFFECT;

	void reset();
	void updatePeaks(int mode);
	void invalidateAllPeaks();
	BufferBox _cdecl readBuffers(int level_no, const Range &r);
	BufferBox _cdecl readBuffersCol(const Range &r);

	string _cdecl getNiceName();
	int get_index();

	// actions
	void _cdecl setName(const string &name);
	void _cdecl setMuted(bool muted);
	void _cdecl setVolume(float volume);
	void _cdecl setPanning(float panning);
	BufferBox _cdecl getBuffers(int level_no, const Range &r);
	void _cdecl insertMidiData(int offset, MidiData &midi);
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
	void _cdecl addMidiEvent(const MidiEvent &e);
	void _cdecl addMidiEvents(const MidiData &events);
	void _cdecl deleteMidiEvent(int index);
	void _cdecl setSynthesizer(Synthesizer *synth);
	void _cdecl editSynthesizer(const string &param_old);
	void _cdecl addBars(int index, float bpm, int beats, int bars);
	void _cdecl addPause(int index, float time);
	void _cdecl editBar(int index, BarPattern &p);
	void _cdecl deleteBar(int index);


	enum
	{
		TYPE_AUDIO,
		TYPE_TIME,
		TYPE_MIDI
	};

// data
	int type;
	string name;

	Array<TrackLevel> level;

	float volume, panning;
	bool muted;

	Array<Effect*> fx;
	Array<SampleRef*> sample;

	// time track
	BarCollection bar;

	// midi track
	MidiData midi;
	Synthesizer *synth;

	AudioFile *root;

	bool is_selected;
};

#endif /* TRACK_H_ */
