/*
 * Sample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Sample.h"
#include "SampleRef.h"
#include "base.h"
#include "../lib/math/math.h"
#include "Song.h"
#include <assert.h>

const string Sample::MESSAGE_CHANGE_BY_ACTION = "ChangeByAction";
const string Sample::MESSAGE_REFERENCE = "Reference";
const string Sample::MESSAGE_UNREFERENCE= "Unreference";

Sample::Sample(SignalType _type) {
	owner = nullptr;
	type = _type;

	volume = 1;
	offset = 0;

	ref_count = 0;
	auto_delete = false;

	uid = randi(0x7fffffff);

	buf = nullptr;
	if (_type == SignalType::AUDIO)
		buf = new AudioBuffer;

	_pointer_ref_count = 0;
}

Sample::Sample(const string &_name, const AudioBuffer &_buf) : Sample(SignalType::AUDIO) {
	name = _name;
	*buf = _buf;
	buf->offset = 0;
}

Sample::Sample(const string &_name, const MidiNoteBuffer &_buf) : Sample(SignalType::MIDI) {
	name = _name;
	midi = _buf;
	midi.sort();
}

Sample::~Sample() {
	notify(MESSAGE_DELETE);
	if (buf)
		delete buf;
}

void Sample::__init__(const string &_name, const AudioBuffer &_buf) {
	new(this) Sample(_name, _buf);
}


int Sample::get_index() const {
	if (!owner)
		return -1;
	foreachi(Sample *s, owner->samples, i)
		if (this == s)
			return i;
	return -1;
}

Range Sample::range() const {
	if (type == SignalType::MIDI)
		return Range(0, midi.samples);
	return buf->range0();
}

void Sample::ref() {
	ref_count ++;
	notify(MESSAGE_REFERENCE);
}

void Sample::unref() {
	ref_count --;
	notify(MESSAGE_UNREFERENCE);
}

SampleRef *Sample::create_ref() {
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
	tags.add(Tag(key, value));
}


Sample *Sample::_pointer_ref() {
	_pointer_ref_count ++;
	return this;
}

void Sample::_pointer_unref() {
	_pointer_ref_count --;
	if (_pointer_ref_count == 0)
		delete this;
}

void Sample::set_owner(Song *s) {
	assert(s);
	if (owner) {
		owner = s;
	} else {
		_pointer_ref();
		owner = s;
	}
}

void Sample::unset_owner() {
	if (owner)
		_pointer_unref();
	owner = nullptr;
}

