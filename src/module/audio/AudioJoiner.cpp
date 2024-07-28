/*
 * AudioJoiner.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioJoiner.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../data/base.h"

namespace tsunami {

AudioJoiner::AudioJoiner() :
	Module(ModuleCategory::Plumbing, "AudioJoiner")
{
}

int AudioJoiner::read_audio(int port, AudioBuffer& buf) {
	bool first = true;
	int result = Return::NoSource;
	for (auto _in: port_in) {
		if (!_in->source)
			continue;
		if (first) {
			result = _in->source->read_audio(buf);
			if (result <= 0)
				return result;
			first = false;
		} else {
			// hmmm needs buffering if a has data, but b has none yet (input stream)
			AudioBuffer buf_b;
			buf_b.resize(buf.length);
			int r = _in->source->read_audio(buf_b);
			if (r <= 0)
				return r;
			buf.add(buf_b, 0, 1);
			result = max(result, r);
		}
	}
	return result;
}

}
