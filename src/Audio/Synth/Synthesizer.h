/*
 * Synthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SYNTHESIZER_H_
#define SYNTHESIZER_H_

#include "../../lib/base/base.h"
#include "../../Plugins/Configurable.h"
#include "../../Data/MidiData.h"
#include "../../Data/Instrument.h"

class Range;
class BufferBox;
class MidiSource;
class PluginManager;
class DetuneSynthesizerDialog;
class ActionTrackDetuneSynthesizer;
class FileChunkSynthesizer;
class FileChunkSynthesizerTuning;

class Synthesizer : public Configurable
{
	friend class PluginManager;
	friend class DetuneSynthesizerDialog;
	friend class ActionTrackDetuneSynthesizer;
	friend class FileChunkSynthesizer;
	friend class FileChunkSynthesizerTuning;
public:
	Synthesizer();
	virtual ~Synthesizer();
	void __init__();
	virtual void __delete__();

	virtual void render(BufferBox &buf){}

	int sample_rate;
	void setSampleRate(int sample_rate);

	void update_delta_phi();

	void setInstrument(Instrument &i);

	int keep_notes;

	virtual int read(BufferBox &buf);

	void feed(const MidiRawData &events);
	void endAllNotes();
	void resetMidiData();
	void reset();

	void iterateEvents(int samples);

	bool hasEnded();
	bool auto_stop;

	bool isDefault();

protected:

	Instrument instrument;

	MidiSource *source;

	MidiRawData events;

	Set<int> active_pitch;
	Array<int> delete_me;
	void enablePitch(int pitch, bool enable);

	struct Tuning
	{
		float freq[MAX_PITCH];
		void set_default();
		bool is_default();
	}tuning;
	float delta_phi[MAX_PITCH];

	void lock();
	void unlock();
	bool locked;
};

Synthesizer *CreateSynthesizer(const string &name);
Array<string> FindSynthesizers();

#endif /* SYNTHESIZER_H_ */
