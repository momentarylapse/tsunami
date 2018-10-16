/*
 * AudioJoiner.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioJoiner.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Data/base.h"

AudioJoiner::AudioJoiner() :
	Module(ModuleType::AUDIO_JOINER)
{
	out = new Output(this);
	port_out.add(out);
	port_in.add(InPortDescription(SignalType::AUDIO, (Port**)&a, "a"));
	port_in.add(InPortDescription(SignalType::AUDIO, (Port**)&b, "b"));
	a = nullptr;
	b = nullptr;
}

int AudioJoiner::Output::read(AudioBuffer& buf)
{
	if (joiner->a and joiner->b){
		int ra = joiner->a->read(buf);
		if (ra <= 0)
			return ra;
		// hmmm needs buffering if a has data, but b has none yet (input stream)
		AudioBuffer buf_b;
		buf_b.resize(buf.length);
		int rb = joiner->b->read(buf_b);
		buf.add(buf_b, 0, 1, 0);
		return max(ra, rb);
	}else if (joiner->a){
		return joiner->a->read(buf);
	}else if (joiner->b){
		return joiner->b->read(buf);
	}
	return buf.length;
}

AudioJoiner::Output::Output(AudioJoiner *j) : AudioPort("out")
{
	joiner = j;
}

void AudioJoiner::Output::reset()
{
	if (joiner->a)
		joiner->a->reset();
	if (joiner->b)
		joiner->b->reset();
}

int AudioJoiner::Output::get_pos(int delta)
{
	if (joiner->a)
		return joiner->a->get_pos(delta);
	if (joiner->b)
		return joiner->b->get_pos(delta);
	return 0;
}
