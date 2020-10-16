/*
 * Sample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleRef.h"
#include "Sample.h"
#include "TrackLayer.h"
#include "../lib/math/math.h"
#include "Song.h"

const string SampleRef::MESSAGE_CHANGE_BY_ACTION = "ChangeByAction";



SampleRef::SampleRef(shared<Sample> sample) {
	//msg_write("  new SampleRef " + p2s(this));
	origin = sample;
	origin->ref();
	layer = nullptr;
	owner = nullptr;
	pos = 0;
	volume = 1;
	muted = false;
}

SampleRef::~SampleRef() {
	notify(MESSAGE_DELETE);
	origin->unref();
	//msg_write("  del SampleRef " + p2s(this));
}

void SampleRef::__init__(shared<Sample> sam) {
	new(this) SampleRef(sam);
}

void SampleRef::__delete__() {
	this->SampleRef::~SampleRef();
}

int SampleRef::get_index() const {
	if (layer) {
		return weak(layer->samples).find((SampleRef*)this);
	}
	return -1;
}

/*Track *SampleRef::track() const {
	if (owner)
		return owner->tracks[track_no];
	return NULL;
}*/

Range SampleRef::range() const {
	return origin.get()->range() + pos;
}

SignalType SampleRef::type() const {
	return origin.get()->type;
}

AudioBuffer& SampleRef::buf() const {
	return *origin.get()->buf;
}

MidiNoteBuffer& SampleRef::midi() const {
	return origin.get()->midi;
}
