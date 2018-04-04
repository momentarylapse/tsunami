/*
 * MidiPort.cpp
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#include "MidiPort.h"


MidiPort::MidiPort()
{
}

void MidiPort::__init__()
{
	new(this) MidiPort;
}

void MidiPort::__delete__()
{
	this->MidiPort::~MidiPort();
}

