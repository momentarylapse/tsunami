/*
 * AudioJoiner.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioJoiner.h"

AudioJoiner::AudioJoiner(AudioSource* _a, AudioSource* _b)
{
	a = _a;
	b = _b;
}

AudioJoiner::~AudioJoiner()
{
}

int AudioJoiner::read(AudioBuffer& buf)
{
	if (a and b){
		return a->read(buf);
	}else if (a){
		return a->read(buf);
	}else if (b){
		return b->read(buf);
	}
	return buf.length;
}

void AudioJoiner::reset()
{
	if (a)
		a->reset();
	if (b)
		b->reset();
}

int AudioJoiner::get_pos(int delta)
{
	if (a)
		return a->get_pos(delta);
	if (b)
		return b->get_pos(delta);
	return 0;
}

int AudioJoiner::sample_rate()
{
	if (a)
		return a->sample_rate();
	if (b)
		return b->sample_rate();
	return DEFAULT_SAMPLE_RATE;
}

void AudioJoiner::set_source_a(AudioSource* _a)
{
	a = _a;
}

void AudioJoiner::set_source_b(AudioSource* _b)
{
	b = _b;
}
