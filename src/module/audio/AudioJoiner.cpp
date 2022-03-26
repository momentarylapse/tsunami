/*
 * AudioJoiner.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioJoiner.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../data/base.h"

AudioJoiner::AudioJoiner() :
	Module(ModuleCategory::PLUMBING, "AudioJoiner")
{
	port_out.add(new Output(this));
	port_in.add({SignalType::AUDIO, &a, "a"});
	port_in.add({SignalType::AUDIO, &b, "b"});
	a = nullptr;
	b = nullptr;
}

int AudioJoiner::Output::read_audio(AudioBuffer& buf) {
	if (joiner->a and joiner->b) {
		int ra = joiner->a->read_audio(buf);
		if (ra <= 0)
			return ra;
		// hmmm needs buffering if a has data, but b has none yet (input stream)
		AudioBuffer buf_b;
		buf_b.resize(buf.length);
		int rb = joiner->b->read_audio(buf_b);
		buf.add(buf_b, 0, 1);
		return max(ra, rb);
	} else if (joiner->a) {
		return joiner->a->read_audio(buf);
	} else if (joiner->b) {
		return joiner->b->read_audio(buf);
	}
	return NO_SOURCE;
}

AudioJoiner::Output::Output(AudioJoiner *j) : Port(SignalType::AUDIO, "out") {
	joiner = j;
}
