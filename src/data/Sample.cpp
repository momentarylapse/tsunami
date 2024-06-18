/*
 * Sample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Sample.h"
#include "SampleRef.h"
#include "base.h"
#include "../lib/base/algo.h"
#include "Song.h"
#include <assert.h>


Sample::Sample(SignalType _type) {
	//msg_write("  new Sample " + p2s(this));
	owner = nullptr;
	type = _type;

	volume = 1;
	offset = 0;

	ref_count = 0;
	auto_delete = false;

	uid = create_uid();

	buf = nullptr;
	if (_type == SignalType::Audio)
		buf = new AudioBuffer;
}

Sample::Sample(const string &_name, const AudioBuffer &_buf) : Sample(SignalType::Audio) {
	name = _name;
	*buf = _buf;
	buf->offset = 0;
}

Sample::Sample(const string &_name, const MidiNoteBuffer &_buf) : Sample(SignalType::Midi) {
	name = _name;
	midi = _buf;
	midi.sort();
}

Sample::~Sample() {
	out_death.notify();
	if (buf)
		delete buf;
	//msg_write("  del Sample " + p2s(this));
}

void Sample::__init__(const string &_name, const AudioBuffer &_buf) {
	new(this) Sample(_name, _buf);
}


int Sample::get_index() const {
	if (!owner)
		return -1;
	return base::find_index(weak(owner->samples), const_cast<Sample*>(this));
}

Range Sample::range() const {
	if (type == SignalType::Midi)
		return Range(0, midi.samples);
	return buf->range0();
}

void Sample::ref() {
	ref_count ++;
	out_reference.notify();
}

void Sample::unref() {
	ref_count --;
	out_unreference.notify();
}

xfer<SampleRef> Sample::create_ref() {
	return new SampleRef(this);
}

string Sample::get_value(const string &key) const {
	for (Tag &t: tags)
		if (t.key == key)
			return t.value;
	return "";
}

void Sample::set_value(const string &key, const string &value) {
	for (Tag &t: tags)
		if (t.key == key) {
			t.value = value;
			return;
		}
	tags.add({key, value});
}

void Sample::set_owner(Song *s) {
	assert(s);
	owner = s;
}

void Sample::unset_owner() {
	owner = nullptr;
}

int Sample::create_uid() {
	return randi(0x7fffffff);
}

