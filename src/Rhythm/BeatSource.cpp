/*
 * BeatSource.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "BeatSource.h"
#include "../Session.h"
#include "../Plugins/PluginManager.h"
#include "../Plugins/Plugin.h"

DummyBeatSource* BeatSource::dummy = new DummyBeatSource;

BeatSource::BeatSource() :
	Module(Session::GLOBAL, Type::BEAT_SOURCE)
{
	out = new Output(this);
}

BeatSource::~BeatSource()
{
	delete out;
}

void BeatSource::__init__()
{
	new(this) BeatSource;
}

void BeatSource::__delete__()
{
	this->BeatSource::~BeatSource();
}

BeatSource::Output::Output(BeatSource* s)
{
	source = s;
}

int BeatSource::Output::read(Array<Beat> &beats, int samples)
{
	return source->read(beats, samples);
}

void BeatSource::Output::reset()
{
	source->reset();
}




// TODO: move to PluginManager?
BeatSource *CreateBeatSource(Session *session, const string &name)
{
	Plugin *p = session->plugin_manager->GetPlugin(session, Plugin::Type::BEAT_SOURCE, name);
	BeatSource *s = NULL;
	if (p->usable)
		s = (BeatSource*)p->create_instance(session, "BeatSource");

	// dummy?
	if (!s)
		s = new BeatSource;

	s->name = name;
	s->plugin = p;
	s->usable = p->usable;
	s->song = session->song;
	s->session = session;
	s->reset_config();
	return s;
}

