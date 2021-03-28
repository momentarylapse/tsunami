/*
 * AudioChannelSelector.cpp
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#include "AudioChannelSelector.h"
#include "../../Data/base.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Stuff/BackupManager.h"
#include "../../lib/math/math.h"

AudioChannelSelector::AudioChannelSelector() : Module(ModuleCategory::PLUMBING, "AudioChannelSelector") {
	out = new Output(this);
	port_out.add(out);
	port_in.add({SignalType::AUDIO, &source, "in"});
	source = nullptr;
}

int AudioChannelSelector::Output::read_audio(AudioBuffer& buf) {
	if (!cs->source)
		return buf.length;

	AudioBuffer buf_in;
	buf_in.resize(buf.length);
	int r = cs->source->read_audio(buf_in);

	if (r > 0)
		cs->apply(buf_in.ref(0, r), buf);

	return r;
}

AudioChannelSelector::Output::Output(AudioChannelSelector *_cs) : Port(SignalType::AUDIO, "out") {
	cs = _cs;
}

void AudioChannelSelector::set_map(const Array<int> &_map) {
	map = _map;
}

void AudioChannelSelector::apply(const AudioBuffer &buf_in, AudioBuffer &buf_out) {
	for (int o=0; o<buf_out.channels; o++) {
		int i = min(o, buf_in.channels - 1);
		if (o < map.num)
			i = clamp(map[o], 0, buf_in.channels - 1);
		memcpy(&buf_out.c[o][0], &buf_in.c[i][0], sizeof(float) * buf_in.length);
	}
}


