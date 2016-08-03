/*
 * Sample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleRef.h"
#include "Sample.h"
#include "../lib/math/math.h"
#include "Song.h"

const string SampleRef::MESSAGE_CHANGE_BY_ACTION = "ChangeByAction";



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
}

SampleRef::~SampleRef()
{
	notify(MESSAGE_DELETE);
	origin->unref();
}

void SampleRef::__init__(Sample *sam)
{
	new(this) SampleRef(sam);
}

void SampleRef::__delete__()
{
	this->SampleRef::~SampleRef();
}

int SampleRef::get_index() const
{
	Track *t = track();
	if (t){
		foreachi(SampleRef *s, t->samples, i)
			if (this == s)
				return i;
	}
	return -1;
}

Track *SampleRef::track() const
{
	if (owner)
		return owner->tracks[track_no];
	return NULL;
}

Range SampleRef::range() const
{
	return origin->range() + pos;
}

int SampleRef::type() const
{
	return origin->type;
}
