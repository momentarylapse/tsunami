/*
 * BeatSource.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "BeatSource.h"
#include "../ModuleFactory.h"
#include "../../Session.h"
#include "../../data/base.h"

namespace tsunami {

BeatSource::BeatSource() :
	Module(ModuleCategory::BEAT_SOURCE, "")
{
}

void BeatSource::__init__() {
	new(this) BeatSource;
}

void BeatSource::__delete__() {
	this->BeatSource::~BeatSource();
}

int BeatSource::read_beats(int port, Array<Beat> &beats, int samples) {
	return read(beats, samples);
}



BeatSource *CreateBeatSource(Session *session, const string &name) {
	return (BeatSource*)ModuleFactory::create(session, ModuleCategory::BEAT_SOURCE, name);
}

}
