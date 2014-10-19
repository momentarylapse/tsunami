/*
 * Sample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Sample.h"
#include "AudioFile.h"
#include "../lib/math/math.h"

const string SampleRef::MESSAGE_CHANGE_BY_ACTION = "ChangeByAction";

Sample::Sample(int _type)
{
	owner = NULL;
	type = _type;

	volume = 1;
	offset = 0;

	ref_count = 0;
	auto_delete = false;

	uid = randi(0x7fffffff);
}

Sample::~Sample()
{
}


int Sample::get_index()
{
	if (!owner)
		return -1;
	foreachi(Sample *s, owner->sample, i)
		if (this == s)
			return i;
	return -1;
}

void Sample::ref()
{
	ref_count ++;
}

void Sample::unref()
{
	ref_count --;
}

SampleRef *Sample::create_ref()
{
	return new SampleRef(this);
}



SampleRef::SampleRef(Sample *sample) :
	Observable("SampleRef")
{
	buf = &sample->buf;
	midi = &sample->midi;
	origin = sample;
	origin->ref();
	track_no = -1;
	owner = NULL;
	pos = 0;
	volume = 1;
	muted = false;
	rep_num = 0;
	rep_delay = 0;
	if (sample->owner)
		rep_delay = sample->owner->sample_rate;
	is_selected = false;
}

SampleRef::~SampleRef()
{
	origin->unref();
}

void SampleRef::__init__(Sample *sam)
{
	new(this) SampleRef(sam);
}

void SampleRef::__delete__()
{
	this->~SampleRef();
}

int SampleRef::get_index()
{
	Track *t = GetParent();
	if (t){
		foreachi(SampleRef *s, t->sample, i)
			if (this == s)
				return i;
	}
	return -1;
}

Track *SampleRef::GetParent()
{
	if (owner)
		return owner->track[track_no];
	return NULL;
}

Range SampleRef::GetRange()
{
	return Range(pos, buf->num);
}
