/*
 * AudioSource.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../Source/AudioSource.h"

const int AudioSource::NOT_ENOUGH_DATA = 0;
const int AudioSource::END_OF_STREAM = -1;

AudioSource::AudioSource()
{
}

void AudioSource::__init__()
{
	new(this) AudioSource;
}

void AudioSource::__delete__()
{
	this->~AudioSource();
}
