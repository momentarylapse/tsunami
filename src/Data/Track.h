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


class Track
{
public:
	Track();
	virtual ~Track();
	Track *GetParent();
	Range GetRange();
	Range GetRangeUnsafe();

	void Reset();
	void UpdatePeaks(int mode);
	void InvalidateAllPeaks();
	BufferBox ReadBuffers(int level_no, const Range &r);
	BufferBox ReadBuffersCol(const Range &r);

	string GetNiceName();

	// actions
	void SetName(const string &name);
	void SetMuted(bool muted);
	void SetVolume(float volume);
	void SetPanning(float panning);
	BufferBox GetBuffers(int level_no, const Range &r);
	void InsertMidiData(int offset, MidiData &midi);
	void AddEffect(Effect *effect);
	void DeleteEffect(int index);
	void EditEffect(int index, const string &param_old);
	SampleRef *AddSample(int pos, int index);
	void DeleteSample(int index);
	void AddMidiNote(const MidiNote &n);
	void DeleteMidiNote(int index);
	void SetSynthesizer(Synthesizer *synth);
	void AddBars(int index, float bpm, int beats, int bars);
	void AddPause(int index, float time);
	void EditBar(int index, BarPattern &p);
	void DeleteBar(int index);


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

// editing
	rect area;
	AudioFile *root;

	bool is_selected;
};

#endif /* TRACK_H_ */
