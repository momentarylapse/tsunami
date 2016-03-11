/*
 * MidiEvent.cpp
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#include "MidiEvent.h"



MidiEvent::MidiEvent(int _pos, float _pitch, float _volume)
{
	pos = _pos;
	pitch = _pitch;
	volume = _volume;
}

