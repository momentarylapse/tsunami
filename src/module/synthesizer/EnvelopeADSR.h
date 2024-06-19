/*
 * EnvelopeADSR.h
 *
 *  Created on: 03.02.2015
 *      Author: michi
 */

#ifndef SRC_MODULE_SYNTH_ENVELOPEADSR_H_
#define SRC_MODULE_SYNTH_ENVELOPEADSR_H_

#include "../../lib/base/base.h"

namespace tsunami {

class EnvelopeADSR {
public:
	EnvelopeADSR();
	void __init__();

	void set(float t_attack, float t_decay, float sustain, float t_release, int sample_rate);
	void set2(float initial, float peak);
	void reset();

	void start(float volume);
	void end();

	float get();
	Array<float> read(int n);

	enum class Mode {
		Off,
		Attack,
		AttackZombie,
		Decay,
		Sustain,
		Release
	};

	// config
	float step_attack;
	float step_decay;
	float factor_release;
	float initial;
	float peak;
	float sustain;
	int ttl_attack;
	int ttl_decay;
	int ttl_release;

	// state
	Mode mode;
	int ttl;
	float value;
	bool just_killed;
	float value_initial;
	float value_peak;
	float value_sustain;

	void start_attack();
	void start_attack_zombie();
	void start_decay();
	void start_sustain();
	void start_release();
	void kill();
};

}

#endif /* SRC_MODULE_SYNTH_ENVELOPEADSR_H_ */
