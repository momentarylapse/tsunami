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

namespace tsunami {

class DummySynthesizer : public Synthesizer {
public:
	DummySynthesizer();
	void __init__();

	PitchRenderer _cdecl *create_pitch_renderer(int pitch) override;

	void _cdecl on_config() override;

	void _set_drum(int no, float freq, float volume, float attack, float release);

	EnvelopeADSR env[MaxPitch];
};

}

#endif /* SRC_MODULE_SYNTH_DUMMYSYNTHESIZER_H_ */
