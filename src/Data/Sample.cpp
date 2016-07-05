/*
 * Sample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Sample.h"
#include "../lib/math/math.h"
#include "Song.h"

const string Sample::MESSAGE_CHANGE_BY_ACTION = "ChangeByAction";
const string Sample::MESSAGE_REFERENCE = "Reference";
const string Sample::MESSAGE_UNREFERENCE= "Unreference";

Sample::Sample(int _type) :
	Observable("Sample")
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
	notify(MESSAGE_DELETE);
}


int Sample::get_index() const
{
	if (!owner)
		return -1;
	foreachi(Sample *s, owner->samples, i)
		if (this == s)
			return i;
	return -1;
}

Range Sample::getRange() const
{
	if (type == Track::TYPE_MIDI)
		return Range(0, midi.samples);
	return buf.range0();
}

void Sample::ref()
{
	ref_count ++;
	notify(MESSAGE_REFERENCE);
}

void Sample::unref()
{
	ref_count --;
	notify(MESSAGE_UNREFERENCE);
}

SampleRef *Sample::create_ref()
{
	return new SampleRef(this);
}

string Sample::getValue(const string &key) const
{
	for (Tag &t : tags)
		if (t.key == key)
			return t.value;
	return "";
}

