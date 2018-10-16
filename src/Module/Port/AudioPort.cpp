/*
 * AudioPort.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioPort.h"
#include "../../Data/base.h"

AudioPort::AudioPort(const string &name) :
	Port(SignalType::AUDIO, name)
{
}

void AudioPort::__init__(const string &name)
{
	new(this) AudioPort(name);
}

void AudioPort::__delete__()
{
	this->AudioPort::~AudioPort();
}
