/*
 * Synthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SRC_MODULE_SYNTH_SYNTHESIZER_H_
#define SRC_MODULE_SYNTH_SYNTHESIZER_H_

#include "../../lib/base/base.h"
#include "../../lib/base/set.h"
#include "../../data/midi/MidiData.h"
#include "../../data/midi/Instrument.h"
#include "../../data/midi/Temperament.h"
#include "../Module.h"
#include "../port/Port.h"

class Range;
class AudioBuffer;
class PluginManager;
class TemperamentDialog;
class ActionTrackDetuneSynthesizer;
class FileChunkSynthesizer;
class FileChunkSynthesizerTuning;
class Session;
class Synthesizer;
namespace kaba {
	class Class;
}

class PitchRenderer : public VirtualBase {
public:
	PitchRenderer(Synthesizer *synth, int pitch);
	virtual ~PitchRenderer(){}

	void _cdecl __init__(Synthesizer *synth, int pitch);
	void _cdecl __delete__() override;

	virtual bool _cdecl render(AudioBuffer &buf){ return false; }
	virtual void _cdecl on_start(float volume){}
	virtual void _cdecl on_end(){}
	virtual void _cdecl on_config(){}

	int pitch;
	float delta_phi;
	Synthesizer *synth;
};

class Synthesizer : public Module {
	friend class PluginManager;
	friend class TemperamentDialog;
	friend class ActionTrackDetuneSynthesizer;
	friend class FileChunkSynthesizer;
	friend class FileChunkSynthesizerTuning;
	friend class PitchRenderer;
public:
	Synthesizer();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	virtual void _cdecl render(AudioBuffer &buf);
	void on_config() override;

	int sample_rate;
	void _cdecl set_sample_rate(int sample_rate);

	void _cdecl update_delta_phi();

	void _cdecl set_instrument(Instrument &i);

	int keep_notes;
	bool auto_generate_stereo;
	bool render_by_ref;


	void _cdecl reset_default();
	void reset_state() override;

	bool has_run_out_of_data();

	bool source_run_out;

	bool is_default();

	class Output : public Port {
	public:
		Output(Synthesizer *synth);
		Synthesizer *synth;
		int read_audio(AudioBuffer &buf) override;

	};

	Port *source;

protected:

	Instrument instrument;

	MidiEventBuffer events;

	kaba::Class* pitch_renderer_class;
	Array<PitchRenderer*> pitch_renderer;
	PitchRenderer *get_pitch_renderer(int pitch);
	virtual PitchRenderer *create_pitch_renderer(int pitch);
	Set<int> active_pitch; // delayed end
	void _render_part(AudioBuffer &buf, int pitch, int offset, int end);
	void _handle_event(const MidiEvent &e);

	Temperament temperament;
	float delta_phi[MAX_PITCH];
};

Synthesizer* CreateSynthesizer(Session *session, const string &name);

#endif /* SRC_MODULE_SYNTH_SYNTHESIZER_H_ */
