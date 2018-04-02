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
#include "../Source/AudioPort.h"

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

	void _cdecl reset();

	bool hasRunOutOfData();

	bool source_run_out;

	bool isDefault();

	class Output : public AudioPort
	{
	public:
		Output(Synthesizer *synth);
		Synthesizer *synth;
		virtual int _cdecl read(AudioBuffer &buf);
		virtual void _cdecl reset();
		virtual int _cdecl sample_rate();

	};
	Output *out;

	void set_source(MidiSource *source);
	MidiSource *source;

protected:

	Instrument instrument;

	MidiEventBuffer events;

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
};

#endif /* SYNTHESIZER_H_ */
