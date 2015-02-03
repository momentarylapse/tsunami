/*
 * EnvelopeADSR.h
 *
 *  Created on: 03.02.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_SYNTH_ENVELOPEADSR_H_
#define SRC_AUDIO_SYNTH_ENVELOPEADSR_H_

class EnvelopeADSR {
public:

	void set(float t_attack, float t_decay, float sustain, float t_release, int sample_rate);
	void set2(float initial, float peak);
	void reset();

	void start(float volume);
	void end();

	float get();

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
	int mode;
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


	enum
	{
		MODE_OFF,
		MODE_ATTACK,
		MODE_ATTACK_ZOMBIE,
		MODE_DECAY,
		MODE_SUSTAIN,
		MODE_RELEASE
	};
};

#endif /* SRC_AUDIO_SYNTH_ENVELOPEADSR_H_ */
