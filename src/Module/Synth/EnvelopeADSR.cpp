/*
 * EnvelopeADSR.cpp
 *
 *  Created on: 03.02.2015
 *      Author: michi
 */

#include "EnvelopeADSR.h"
#include "../../lib/file/file.h"
#include "../../lib/math/math.h"

EnvelopeADSR::EnvelopeADSR() {
	set(0, 0, 0, 0, 0);
	reset();
}

void EnvelopeADSR::__init__() {
	new(this) EnvelopeADSR();
}

void EnvelopeADSR::set(float t_attack, float t_decay, float _sustain, float t_release, int sample_rate) {
	ttl_attack = t_attack * sample_rate;
	ttl_decay = t_decay * sample_rate;
	ttl_release = t_release * sample_rate;
	sustain = _sustain;
	initial = 0;
	peak = 1;
	//reset();
	//msg_write(format("%f  %d  %f  %d  %f  %d", initial, ttl_attack, peak, ttl_decay, sustain, ttl_release));
}

void EnvelopeADSR::set2(float _initial, float _peak) {
	initial = _initial;
	peak = _peak;
	//msg_write(format("%f  %d  %f  %d  %f  %d", initial, ttl_attack, peak, ttl_decay, sustain, ttl_release));
}

void EnvelopeADSR::reset() {
	mode = MODE_OFF;
	value = 0;
	just_killed = false;
}

void EnvelopeADSR::start(float volume) {
	value_initial = volume * initial;
	value_peak = volume * peak;
	value_sustain = volume * sustain;
	just_killed = false;

	step_attack = 0;
	step_decay = 0;
	factor_release = 0;
	if (ttl_attack > 0)
		step_attack = (value_peak - value) / (float)ttl_attack;
	if (ttl_decay > 0)
		step_decay = (value_sustain - value_peak) / (float)ttl_decay;
	if (ttl_release > 0)
		factor_release = exp( - 3.0f / (float)ttl_release);


	start_attack();
}

void EnvelopeADSR::end() {
	if (mode == MODE_ATTACK)
		start_attack_zombie();
	else
		start_release();
}

float EnvelopeADSR::get() {
	if (mode == MODE_ATTACK) {
		value += step_attack;
		ttl --;
		if (ttl <= 0)
			start_decay();
	} else if (mode == MODE_ATTACK_ZOMBIE) {
		value += step_attack;
		ttl --;
		if (ttl <= 0)
			start_release();
	} else if (mode == MODE_DECAY) {
		value += step_decay;
		ttl --;
		if (ttl <= 0)
			start_sustain();
	} else if (mode == MODE_RELEASE) {
		value *= factor_release;
		if (value <= 0.0001f)
			kill();
	}
	return value;
}

Array<float> EnvelopeADSR::read(int n) {
	Array<float> f;
	f.resize(n);
	for (int i=0; i<n; i++)
		f[i] = get();
	return f;
}

void EnvelopeADSR::start_attack() {
	if (value < value_initial)
		value = value_initial;

	mode = MODE_ATTACK;
	ttl = ttl_attack;
	just_killed = false;

	if (ttl <= 0)
		start_decay();
}

void EnvelopeADSR::start_attack_zombie() {
	mode = MODE_ATTACK_ZOMBIE;
}

void EnvelopeADSR::start_decay() {
	value = value_peak;
	mode = MODE_DECAY;
	ttl = ttl_decay;

	if (ttl <= 0)
		start_sustain();
}

void EnvelopeADSR::start_sustain() {
	value = value_sustain;
	mode = MODE_SUSTAIN;
}

void EnvelopeADSR::start_release() {
	//value = value_sustain;
	mode = MODE_RELEASE;
	ttl = ttl_release;
}

void EnvelopeADSR::kill() {
	value = 0;
	mode = MODE_OFF;
	just_killed = true;
}
