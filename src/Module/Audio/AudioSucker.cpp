/*
 * AudioSucker.cpp
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#include "AudioSucker.h"
#include "../Port/Port.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../lib/file/msg.h"


AudioSucker::AudioSucker() :
	Module(ModuleCategory::PLUMBING, "AudioSucker")
{
	port_in.add({SignalType::AUDIO, &source, "in"});
	source = nullptr;
	channels = 2;
}

int AudioSucker::do_suck(int buffer_size) {
	AudioBuffer temp;
	temp.set_channels(channels);
	temp.resize(buffer_size);
	return source->read_audio(temp);
}

void AudioSucker::set_channels(int _channels) {
	channels = _channels;
}

int AudioSucker::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::SUCK)
		return do_suck(param);
	return COMMAND_NOT_HANDLED;
}

