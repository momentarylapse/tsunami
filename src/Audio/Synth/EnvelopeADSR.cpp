/*
 * EnvelopeADSR.cpp
 *
 *  Created on: 03.02.2015
 *      Author: michi
 */

#include "EnvelopeADSR.h"
#include "../../lib/file/file.h"
#include "../../lib/math/math.h"

void EnvelopeADSR::set(float t_attack, float t_decay, float _sustain, float t_release, int sample_rate)
{
	ttl_attack = t_attack * sample_rate;
	ttl_decay = t_decay * sample_rate;
	ttl_release = t_release * sample_rate;
	sustain = _sustain;
}

void EnvelopeADSR::reset()
{
	mode = MODE_OFF;
	value = 0;
}

void EnvelopeADSR::start(float volume)
{
	step_attack = 0;
	if (ttl_attack > 0)
		step_attack = (volume - value) / (float)ttl_attack;
	step_decay = 0;
	if (ttl_decay > 0)
		step_decay = volume * (sustain - 1) / (float)ttl_decay;
	factor_release = exp( - 1.0f / (float)ttl_release);


	if (ttl_attack > 0){
		mode = MODE_ATTACK;
		ttl = ttl_attack;
	}else if (ttl_decay > 0){
		value = volume;
		mode = MODE_DECAY;
		ttl = ttl_decay;
	}else{
		mode = MODE_SUSTAIN;
		value = volume;
	}
}

void EnvelopeADSR::end()
{
	if (mode == MODE_ATTACK){
		mode = MODE_ATTACK_ZOMBIE;
	}else{
		mode = MODE_RELEASE;
		ttl = ttl_release;
	}
}

float EnvelopeADSR::get()
{
	if (mode == MODE_ATTACK){
		value += step_attack;
		ttl --;
		if (ttl <= 0){
			if (ttl_decay > 0){
				mode = MODE_DECAY;
				ttl = ttl_decay;
			}else{
				mode = MODE_SUSTAIN;
				value *= sustain;
			}
		}
	}else if (mode == MODE_ATTACK_ZOMBIE){
		value += step_attack;
		ttl --;
		if (ttl <= 0){
			mode = MODE_RELEASE;
			ttl = ttl_release;
		}
	}else if (mode == MODE_DECAY){
		value += step_decay;
		ttl --;
		if (ttl <= 0)
			mode = MODE_SUSTAIN;
	}else if (mode == MODE_RELEASE){
		value *= factor_release;
		if (value <= 0.0001f){
			value = 0;
			mode = MODE_OFF;
		}
	}
	return value;
}
