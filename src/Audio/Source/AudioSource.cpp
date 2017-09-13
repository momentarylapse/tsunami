/*
 * AudioSource.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../Source/AudioSource.h"

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

int AudioSource::readResize(BufferBox &buf)
{
	int n = read(buf);
	buf.resize(n);
	return n;
}

Array<Tag> AudioSource::getTags()
{
	Array<Tag> tags;
	return tags;
}
