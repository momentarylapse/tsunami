/*
 * MidiPort.cpp
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#include "MidiPort.h"


const int MidiPort::NOT_ENOUGH_DATA = 0;
const int MidiPort::END_OF_STREAM = -2;

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

