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
#include "../../Midi/MidiData.h"
#include "../../Midi/Instrument.h"

class Range;
class AudioBuffer;
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
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual void _cdecl render(AudioBuffer &buf){}

	int sample_rate;
	void _cdecl setSampleRate(int sample_rate);

	void _cdecl update_delta_phi();

	void _cdecl setInstrument(Instrument &i);

	int keep_notes;

	int _cdecl read(AudioBuffer &buf, MidiSource *source);

	void _cdecl reset();

	bool hasRunOutOfData();

	bool source_run_out;

	bool isDefault();

protected:

	Instrument instrument;

	MidiRawData events;

	Set<int> active_pitch;
	Array<int> delete_me;
	void enablePitch(int pitch, bool enable);

public:
	struct Tuning
	{
		float freq[MAX_PITCH];
		void set_default();
		bool is_default();
	};
protected:
	Tuning tuning;
	float delta_phi[MAX_PITCH];

	void lock();
	void unlock();
	bool locked;
};

#endif /* SYNTHESIZER_H_ */
