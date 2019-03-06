/*
 * BeatSource.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "BeatSource.h"
#include "../ModuleFactory.h"
#include "../../Session.h"
#include "../../Data/base.h"

BeatSource::BeatSource() :
	Module(ModuleType::BEAT_SOURCE, "")
{
	out = new Output(this);
	port_out.add(out);
}

void BeatSource::__init__()
{
	new(this) BeatSource;
}

void BeatSource::__delete__()
{
	this->BeatSource::~BeatSource();
}

BeatSource::Output::Output(BeatSource* s) : Port(SignalType::BEATS, "out")
{
	source = s;
}

int BeatSource::Output::read_beats(Array<Beat> &beats, int samples)
{
	return source->read(beats, samples);
}



BeatSource *CreateBeatSource(Session *session, const string &name)
{
	return (BeatSource*)ModuleFactory::create(session, ModuleType::BEAT_SOURCE, name);
}

