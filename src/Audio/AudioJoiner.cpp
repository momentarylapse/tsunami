/*
 * AudioJoiner.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioJoiner.h"

AudioJoiner::AudioJoiner(AudioPort* _a, AudioPort* _b)
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
		int ra = a->read(buf);
		AudioBuffer buf_b;
		buf_b.resize(buf.length);
		int rb = b->read(buf_b);
		buf.add(buf_b, 0, 1, 0);
		return max(ra, rb);
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

void AudioJoiner::set_source_a(AudioPort* _a)
{
	a = _a;
}

void AudioJoiner::set_source_b(AudioPort* _b)
{
	b = _b;
}
