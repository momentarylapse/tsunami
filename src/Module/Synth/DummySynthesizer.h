/*
 * DummySynthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SRC_MODULE_SYNTH_DUMMYSYNTHESIZER_H_
#define SRC_MODULE_SYNTH_DUMMYSYNTHESIZER_H_

#include "Synthesizer.h"
#include "EnvelopeADSR.h"

class DummySynthesizer : public Synthesizer
{
public:
	DummySynthesizer();
	virtual ~DummySynthesizer();
	void __init__();

	bool _cdecl render_pitch(AudioBuffer &buf, int pitch) override;
	void _cdecl handle_event(MidiEvent &event) override;

	void _cdecl on_config() override;
	void _cdecl reset_state() override;

	struct PitchState{
		float volume;
		float phi;
		EnvelopeADSR env;
	}pitch_state[MAX_PITCH];

	void _set_drum(int no, float freq, float volume, float attack, float release);
};

#endif /* SRC_MODULE_SYNTH_DUMMYSYNTHESIZER_H_ */
