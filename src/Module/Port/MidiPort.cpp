/*
 * MidiPort.cpp
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#include "MidiPort.h"
#include "../../Data/base.h"


MidiPort::MidiPort(const string &name) :
	Port(SignalType::MIDI, name)
{
}

void MidiPort::__init__(const string &name)
{
	new(this) MidiPort(name);
}

void MidiPort::__delete__()
{
	this->MidiPort::~MidiPort();
}

