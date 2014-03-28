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
	Observable("SampleRef"),
	buf(sample->buf)
{
	origin = sample;
	origin->ref();
	track_no = -1;
	owner = NULL;
	pos = 0;
	volume = 1;
	muted = false;
	rep_num = 0;
	rep_delay = sample->owner->sample_rate;
	is_selected = false;
}

SampleRef::~SampleRef()
{
	origin->unref();
	Notify("Delete");
}

Track *SampleRef::GetParent()
{
	return owner->track[track_no];
}

Range SampleRef::GetRange()
{
	return Range(pos, buf.num);
}
