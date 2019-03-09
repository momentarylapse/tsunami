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


AudioSucker::AudioSucker() :
	Module(ModuleType::PLUMBING, "AudioSucker")
{
	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));
	source = nullptr;

}

AudioSucker::~AudioSucker()
{
}

int AudioSucker::do_suck(int buffer_size)
{
	AudioBuffer temp;
	temp.resize(buffer_size);
	int r = source->read_audio(temp);
	if (r == source->NOT_ENOUGH_DATA)
		return r;
	if (r == source->END_OF_STREAM)
		return r;
	return r;
}

int AudioSucker::command(ModuleCommand cmd, int param)
{
	if (cmd == ModuleCommand::SUCK)
		return do_suck(param);
	return COMMAND_NOT_HANDLED;
}

