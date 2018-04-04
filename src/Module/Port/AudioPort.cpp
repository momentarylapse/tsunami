/*
 * AudioPort.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioPort.h"

AudioPort::AudioPort()
{
}

void AudioPort::__init__()
{
	new(this) AudioPort;
}

void AudioPort::__delete__()
{
	this->AudioPort::~AudioPort();
}
