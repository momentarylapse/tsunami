/*
 * AudioPort.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioPort.h"

const int AudioPort::NOT_ENOUGH_DATA = 0;
const int AudioPort::END_OF_STREAM = -1;

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
