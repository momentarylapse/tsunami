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
	Range _cdecl GetRange();
	Range GetRangeUnsafe();

	static const string MESSAGE_ADD_EFFECT;
	static const string MESSAGE_DELETE_EFFECT;
	static const string MESSAGE_ADD_MIDI_EFFECT;
	static const string MESSAGE_DELETE_MIDI_EFFECT;

	void Reset();
	void UpdatePeaks(int mode);
	void InvalidateAllPeaks();
	BufferBox _cdecl ReadBuffers(int level_no, const Range &r);
	BufferBox _cdecl ReadBuffersCol(const Range &r);

	string _cdecl GetNiceName();
	int get_index();

	// actions
	void _cdecl SetName(const string &name);
	void _cdecl SetMuted(bool muted);
	void _cdecl SetVolume(float volume);
	void _cdecl SetPanning(float panning);
	BufferBox _cdecl GetBuffers(int level_no, const Range &r);
	void _cdecl InsertMidiData(int offset, MidiData &midi);
	void _cdecl AddEffect(Effect *effect);
	void _cdecl DeleteEffect(int index);
	void _cdecl EditEffect(int index, const string &param_old);
	void _cdecl EnableEffect(int index, bool enabled);
	void _cdecl AddMidiEffect(MidiEffect *effect);
	void _cdecl DeleteMidiEffect(int index);
	void _cdecl EditMidiEffect(int index, const string &param_old);
	void _cdecl EnableMidiEffect(int index, bool enabled);
	SampleRef *_cdecl AddSample(int pos, int index);
	void _cdecl DeleteSample(int index);
	void _cdecl EditSample(int index, float volume, bool mute, int rep_num, int rep_delay);
	void _cdecl AddMidiNote(const MidiNote &n);
	void _cdecl AddMidiNotes(Array<MidiNote> notes);
	void _cdecl DeleteMidiNote(int index);
	void _cdecl SetSynthesizer(Synthesizer *synth);
	void _cdecl EditSynthesizer(const string &param_old);
	void _cdecl AddBars(int index, float bpm, int beats, int bars);
	void _cdecl AddPause(int index, float time);
	void _cdecl EditBar(int index, BarPattern &p);
	void _cdecl DeleteBar(int index);


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
