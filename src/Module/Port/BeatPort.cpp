/*
 * BeatPort.cpp
 *
 *  Created on: 03.04.2018
 *      Author: michi
 */

#include "BeatPort.h"
#include "../../Data/base.h"

BeatPort::BeatPort(const string &name) :
	Port(SignalType::BEATS, name)
{}

void BeatPort::__init__(const string &name)
{
	new(this) BeatPort(name);
}

void BeatPort::__delete__()
{
	this->BeatPort::~BeatPort();
}

