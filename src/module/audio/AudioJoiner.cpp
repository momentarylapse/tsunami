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
}

int AudioJoiner::Output::read_audio(AudioBuffer& buf) {
	auto pa = joiner->in_a.source;
	auto pb = joiner->in_b.source;
	if (pa and pb) {
		int ra = pa->read_audio(buf);
		if (ra <= 0)
			return ra;
		// hmmm needs buffering if a has data, but b has none yet (input stream)
		AudioBuffer buf_b;
		buf_b.resize(buf.length);
		int rb = pb->read_audio(buf_b);
		buf.add(buf_b, 0, 1);
		return max(ra, rb);
	} else if (pa) {
		return pa->read_audio(buf);
	} else if (pb) {
		return pb->read_audio(buf);
	}
	return NO_SOURCE;
}

AudioJoiner::Output::Output(AudioJoiner *j) : Port(SignalType::AUDIO, "out") {
	joiner = j;
}
