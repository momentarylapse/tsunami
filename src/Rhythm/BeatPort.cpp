/*
 * BeatPort.cpp
 *
 *  Created on: 03.04.2018
 *      Author: michi
 */

#include "BeatPort.h"

void BeatPort::__init__()
{
	new(this) BeatPort;
}

void BeatPort::__delete__()
{
	this->BeatPort::~BeatPort();
}

