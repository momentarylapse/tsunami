/*
 * Sample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Sample.h"
#include "AudioFile.h"

Sample::Sample()
{
	owner = NULL;

	volume = 1;
	offset = 0;

	ref_count = 0;
	auto_delete = false;
}

Sample::~Sample()
{
}

void Sample::ref()
{
	ref_count ++;
}

void Sample::unref()
{
	ref_count --;
}



SampleRef::SampleRef(Sample *sample) :
	buf(sample->buf)
{
	origin = sample;
	origin->ref();
	parent = -1;
	pos = 0;
	rep_num = 0;
	rep_delay = 10000;
}

SampleRef::~SampleRef()
{
	origin->unref();
}

Track *SampleRef::GetParent()
{
	return root->track[parent];
}

Range SampleRef::GetRange()
{
	return Range(pos, buf.num);
}
